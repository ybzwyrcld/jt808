// MIT License
//
// Copyright (c) 2020 Yuming Meng
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// @File    :  client.cc
// @Version :  1.0
// @Time    :  2020/07/17 10:44:34
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "client.h"

#include <string.h>
#if defined(__linux__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include <chrono>

#include "socket_util.h"


namespace libjt808 {

namespace {

// 默认的终端注册信息.
const RegisterInfo kRegisterInfo = {
  0x002c,
  0x012c,
  std::vector<uint8_t>{'S', 'K', 'O', 'E', 'M'},
  std::vector<uint8_t>{'S', 'K', '9', '1', '5', '1'},
  std::vector<uint8_t>{'0', '0', '0', '0', '0', '1'},
  kBlue,
  "粤B99999",
};

}  // namespace

JT808Client::JT808Client() {
}

JT808Client::~JT808Client() {
}

// 对一些必要的参数设定一个默认值, 防止协议命令生成不完整.
void JT808Client::Init(void) {
  // 初始化命令解析器和命令封装器.
  JT808FrameParserInit(&parser_);
  JT808FramePackagerInit(&packager_);
  // 预设终端手机号.
  parameter_.msg_head.phone_num = std::move("13395279527");
  parameter_.msg_head.msgbody_attr.u16val = 0;  // 消息体属性.
  parameter_.msg_head.msg_flow_num = 0;  // 消息流水号.
  parameter_.register_info = kRegisterInfo;  // 终端注册信息.
  // 设置默认回调函数, 防止外部未设置时调用出错.
  terminal_parameter_callback_ = [] () -> void { return; };
  upgrade_callback_ = [] (uint8_t const& type,
      char const* data, int const& size) -> void { return; };
  // 位置上报相关.
  location_report_inteval_ = 10;  // 10s位置上报时间间隔.
  location_report_immediately_flag_ = 0;  // 立即上报标志清零.
  parameter_.location_info.alarm.value = 0;
  parameter_.location_info.status.value = 0;
  // 通信流程控制.
  is_connected_.store(false);
  is_authenticated_.store(false);
  service_is_running_.store(false);
}

// 与远程服务器建立TCP连接, 并设置socket为非阻塞模式.
int JT808Client::ConnectRemote(void) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<uint16_t>(port_));
#if defined(__linux__)
  addr.sin_addr.s_addr = inet_addr(ip_.c_str());
  client_= socket(AF_INET, SOCK_STREAM, 0);
  if (client_ == -1) {
    printf("%s[%d]: Create socket failed!!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  struct timeval timeout = {0, 500000};
  setsockopt(client_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  setsockopt(client_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#elif defined(_WIN32)
  WSADATA ws_data;
  if (WSAStartup(MAKEWORD(2,2), &ws_data) != 0) {
    return -1;
  }
  client_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_ == INVALID_SOCKET) {
    printf("%s[%d]: Create socket failed!!!\n", __FUNCTION__, __LINE__);
    WSACleanup();
    return -1;
  }
  addr.sin_addr.S_un.S_addr = inet_addr(ip_.c_str());
#endif
  if (Connect(client_, reinterpret_cast<struct sockaddr *>(&addr),
              sizeof(addr)) == -1) {
    printf("%s[%d]: Connect to remote server failed!!!\n",
           __FUNCTION__, __LINE__);
    Close(client_);
#if defined(_WIN32)
    WSACleanup();
#endif
    return -1;
  }
  // 设置非阻塞模式.
#if defined(__linux__)
  int flags = fcntl(client_, F_GETFL, 0);
  fcntl(client_, F_SETFL, flags | O_NONBLOCK);
#elif defined(_WIN32)
  unsigned long ul = 1;
  if (ioctlsocket(client_, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR) {
    printf("%s[%d]: Set socket nonblock failed!!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
#endif
  is_connected_.store(true);
  return 0;
}

// 与JT808服务端进行注册和鉴权操作, 这两项操作通过后才可以与JT808服务端
// 进行合法通信.
int JT808Client::JT808ConnectionAuthentication(void) {
  if (is_connected_ == false) return -1;
  //
  // 注册.
  //
  // 生成并发送注册消息.
  if (PackagingAndSendMessage(kTerminalRegister) < 0) return -1;
  // 从注册应答消息中解析出注册结果和鉴权码.
  parameter_.parse.respone_result = kTerminalHaveBeenRegistered;
  parameter_.parse.authentication_code.clear();
  if (ReceiveAndParseMessage(5) < 0) return -1;
  // 检查注册结果.
  if ((parameter_.parse.msg_head.msg_id != kTerminalRegisterResponse) ||
      (parameter_.parse.respone_result != kRegisterSuccess)) {
    return -1;
  }
  //
  // 鉴权.
  //
  // 生成并发送鉴权消息.
  if (PackagingAndSendMessage(kTerminalAuthentication) < 0) return -1;
  // 从通用应答中解析出鉴权结果.
  parameter_.parse.respone_result = kFailure;
  if (ReceiveAndParseMessage(5) < 0) return -1;
  // 检查鉴权结果.
  if ((parameter_.parse.respone_msg_id != kTerminalAuthentication) ||
      (parameter_.parse.respone_result != kSuccess)) {
    return -1;
  }
  is_authenticated_.store(true);
  return 0;
}

// 在与JT808服务端成功进行连接和鉴权后启动服务线程.
void JT808Client::Run(void) {
  if (!is_connected_ || !is_authenticated_) return;
  service_thread_ = std::thread(&JT808Client::ThreadHandler, this);
  service_thread_.detach();
}

// 停止服务线程并清除TCP连接.
void JT808Client::Stop(void) {
  if (client_ > 0) {
    service_is_running_.store(false);
    Close(client_);
    client_ = 0;
#if defined(_WIN32)
    WSACleanup();
#endif
    is_authenticated_.store(false);
    is_connected_.store(false);
  }
}

// 根据提供的消息ID以及调用前此函数前对参数的设定, 生成对应的JT808格式消息,
// 并通过socket发送到服务端.
int JT808Client::PackagingAndSendMessage(uint32_t const& msg_id) {
  if (!is_connected_) {
    printf("%s[%d]: Invalid connection !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  std::vector<uint8_t> msg;
  parameter_.msg_head.msg_id = msg_id;  // 设置消息ID.
  if (JT808FramePackage(packager_, parameter_, &msg) < 0) {
    printf("%s[%d]: Package message failed !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  ++parameter_.msg_head.msg_flow_num;  // 每正确生成一条命令, 消息流水号增加1.
  if (Send(client_, reinterpret_cast<char*>(msg.data()), msg.size(), 0) <= 0) {
    printf("%s[%d]: Send message failed !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  return 0;
}

// 阻塞地从socket连接中接收一次数据, 然后按照JT808协议进行解析.
int JT808Client::ReceiveAndParseMessage(int const& timeout) {
  if (!is_connected_) {
    printf("%s[%d]: Invalid connection !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  std::vector<uint8_t> msg;
  int ret = -1;
  int timeout_ms = timeout*1000;  // 超时时间, ms.
  auto tp = std::chrono::steady_clock::now();
  std::unique_ptr<char[]> buffer(
      new char[4096], std::default_delete<char[]>());
  while (1) {
    if ((ret = Recv(client_, buffer.get(), 4096, 0)) > 0) {
      msg.assign(buffer.get(), buffer.get()+ret);
      break;
    } else if (ret == 0) {
      printf("%s[%d]: Disconnect !!!\n", __FUNCTION__, __LINE__);
      is_connected_.store(false);
      return -1;
    } else {
      // TODO(mengyuming@hotmail.com): 其它连接错误需处理.
    }
    // 检测超时退出.
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now()-tp).count() >= timeout_ms) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if (msg.empty()) return -1;
  // 解析消息.
  if (JT808FrameParse(parser_, msg, &parameter_) == -1) {
    printf("%s[%d]: Parse message failed !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  return 0;
}

// 服务端通信线程, 解析接收到的命令, 同时自动进行位置信息上报和心跳包的发送.
void JT808Client::ThreadHandler(void) {
  service_is_running_.store(true);
  int ret = -1;
  int64_t report_intv = location_report_inteval_*1000;  // 时间间隔, ms.
  std::unique_ptr<char[]> buffer(
      new char[4096], std::default_delete<char[]>());
  auto report_begin_tp = std::chrono::steady_clock::now();
  auto heartbeat_begin_tp = std::chrono::steady_clock::now();
  auto end_tp = std::chrono::steady_clock::now();
  auto report_time_lag = std::chrono::duration_cast<
        std::chrono::milliseconds>(end_tp - report_begin_tp).count();
  auto heartbeat_time_lag = std::chrono::duration_cast<
        std::chrono::milliseconds>(end_tp - heartbeat_begin_tp).count();
  int64_t heartbeat_intv;
  uint32_t temp;
  // 从终端参数中获取心跳包时间间隔, 若未找到则使用默认60秒(s)心跳.
  if (GetTerminalHeartbeatInterval(&temp) == 0) {
    heartbeat_intv = temp * 1000;
  } else {
    heartbeat_intv = 60000;  // 60s.
  }
  std::vector<uint8_t> msg;
  bool first_report = true;
  while (service_is_running_) {
    if ((ret = Recv(client_, buffer.get(), 4096, 0)) > 0) {
      // printf("Recv[%d]: %s\n", ret, buffer.get());
      // continue;
      msg.assign(buffer.get(), buffer.get()+ret);
      if (JT808FrameParse(parser_, msg, &parameter_) == 0) {
        auto const& msg_id = parameter_.parse.msg_head.msg_id;
        if (msg_id == kSetTerminalParameters) {  // 设置终端参数.
          // 更新终端参数.
          for (auto const& it : parameter_.parse.terminal_parameters) {
            if (parameter_.terminal_parameters.find(it.first) !=
                parameter_.terminal_parameters.end()) {
              parameter_.terminal_parameters[it.first] = it.second;
            } else {
              parameter_.terminal_parameters.insert(it);
            }
          }
          // 调用回调函数.
          terminal_parameter_callback_();
        } else if (msg_id == kGetTerminalParameters ||
                   msg_id == kGetSpecificTerminalParameters) {  // 查询终端参数.
          auto const& ids = parameter_.parse.terminal_parameter_ids;
          if (ids.empty()) {  // 返回全部参数.
            parameter_.terminal_parameter_ids.clear();
          } else {  // 返回指定参数.
            parameter_.terminal_parameter_ids.assign(ids.begin(), ids.end());
          }
          PackagingAndSendMessage(kGetTerminalParametersResponse);
        }
      }
    } else if (ret == 0) {
      printf("%s[%d]: Disconnect !!!\n", __FUNCTION__, __LINE__);
      break;
    } else {  // TODO(mengyuming@hotmail.com): 其它连接错误需处理.
      // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    end_tp = std::chrono::steady_clock::now();
    // 上次发送位置上报消息到此时的时间差.
    report_time_lag = std::chrono::duration_cast<
        std::chrono::milliseconds>(end_tp - report_begin_tp).count();
    // 上次发送心跳包到此时的时间差.
    heartbeat_time_lag = std::chrono::duration_cast<
        std::chrono::milliseconds>(end_tp - heartbeat_begin_tp).count();
    // 到达时间间隔或有立即上报的标志时进行位置信息汇报.
    if (report_time_lag >= report_intv || location_report_immediately_flag_) {
      // 首次上报需要等到成功定位后再进行, 再此期间可以进行心跳包发送.
      if (parameter_.location_info.status.bit.positioning == 0) {
        if (first_report) {
          if (heartbeat_time_lag >= heartbeat_intv) {
            heartbeat_begin_tp = end_tp;
            PackagingAndSendMessage(kTerminalHeartBeat);
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          continue;
        }
      }
      if (first_report) first_report = false;
      location_report_immediately_flag_ = 0;
      report_begin_tp = end_tp;
      heartbeat_begin_tp = end_tp;  // 进行位置汇报后重置心跳检测时间.
      PackagingAndSendMessage(kLocationReport);
    } else if (heartbeat_time_lag >= heartbeat_intv) {
      heartbeat_begin_tp = end_tp;
      PackagingAndSendMessage(kTerminalHeartBeat);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  // 线程终止.
  service_is_running_.store(false);
  Stop();
}

}  // namespace libjt808
