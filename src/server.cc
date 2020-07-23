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

// @File    :  server.cc
// @Version :  1.0
// @Time    :  2020/06/24 09:56:45
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#if defined(__linux__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include <algorithm>
#include <chrono>

#include "socket_util.h"


namespace libjt808 {

namespace {

// 16进制表.
constexpr uint8_t kHexTable[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

// 任意数值转16进制字符串.
template<typename T>
inline std::string ToHexString(T const& t) {
  std::string result;
  const int len = sizeof(t);
  union {
    T t;
    uint8_t array[len];
  } cvt = {t};
  for (int i = 0; i < len; ++i) {
    result.push_back(kHexTable[cvt.array[len-i-1]/16]);
    result.push_back(kHexTable[cvt.array[len-i-1]%16]);
  }
  return result;
}

// 单字节终端参数.
const std::vector<uint32_t> kByteTypeTerminalParameters = {
  kGNSSModelEnable, kCDRadioModelEnable, kNtripCorsEnable,
  kNtripServiceEnable, kJT808ServiceEnable,
  kGNSSLogGGA, kGNSSLogRMC, kGNSSLogATT,
  kCDRaiodReceiceMode, kCDRaiodFormCode, kNtripCorsGGAReportInterval,
  kNtripServiceGGAReportInterval, kJT808ServiceReportInterval,
};
// 双字节终端参数.
const std::vector<uint32_t> kWordTypeTerminalParameters = {
  kCDRaiodWorkFreq, kNtripCorsPort, kNtripServicePort, kJT808ServicePort,
};
// 四字节终端参数.
const std::vector<uint32_t> kDWordTypeTerminalParameters = {
  kTerminalHeartBeatInterval, kCDRaiodServiceBDRT,
};
// 字符串终端参数.
const std::vector<uint32_t> kStringTypeTerminalParameters = {
  kNtripCorsIP, kNtripCorsUser, kNtripCorsPasswd, kNtripCorsMountPoint,
  kNtripServiceIP, kNtripServiceUser, kNtripServicePasswd,
  kNtripServiceMountPoint,
  kJT808ServiceIP, kJT808ServicePhoneNumber,
};

// 显示位置上报信息.
void PrintLocationReportInfo(ProtocolParameter const& para) {
  auto const& basic_info = para.parse.location_info;
  auto const& extension_info = para.parse.location_extension;
  printf("inout area alarm bit: %d\n", basic_info.alarm.bit.in_out_area);
  printf("psoition status: %d\n", basic_info.status.bit.positioning);
  printf("latitude: %.6lf\n", basic_info.latitude*1e-6);
  printf("longitude: %.6lf\n", basic_info.longitude*1e-6);
  printf("atitude: %d\n", basic_info.altitude);
  printf("speed: %d\n", basic_info.speed*10);
  printf("bearing: %d\n", basic_info.bearing);
  printf("time: %s\n", basic_info.time.c_str());
  auto it = extension_info.find(libjt808::kGnssSatellites);
  if (it != extension_info.end())
    printf("satellites number: %d\n", it->second[0]);
  it = extension_info.find(libjt808::kCustomInformationLength);
  if (it != extension_info.end())
    printf("has extension custom\n");
  it = extension_info.find(libjt808::kPositioningStatus);
  if (it != extension_info.end())
    printf("position fix status: %d\n", it->second[0]);
  it = extension_info.find(libjt808::kAccessAreaAlarm);
  if (it != extension_info.end()) {
    uint8_t location_type;
    uint32_t area_toute_id;
    uint8_t direrion;
    if (libjt808::GetAccessAreaAlarmBody(it->second,
        &location_type, &area_toute_id, &direrion) == 0) {
      printf("area type: %d\n", location_type);
      printf("area id: %04X\n", area_toute_id);
      printf("area direction: %d\n", direrion);
    }
  }
}

// 生成终端参数项的格式化输出字符串.
std::string GeneralTerminalParameterFormatString(
    uint32_t const& id, std::vector<uint8_t> const& value) {
  std::string result = "id: ";
  result += ToHexString(id) + ", value: ";
  if (find(kByteTypeTerminalParameters.begin(),
              kByteTypeTerminalParameters.end(), id) !=
          kByteTypeTerminalParameters.end()) {
    result += std::to_string(value[0]);
  } else if (find(kWordTypeTerminalParameters.begin(),
                  kWordTypeTerminalParameters.end(), id) !=
                kWordTypeTerminalParameters.end()) {
    result += std::to_string(value[0]*256+value[1]);
  } else if (find(kDWordTypeTerminalParameters.begin(),
                      kDWordTypeTerminalParameters.end(), id) !=
                 kDWordTypeTerminalParameters.end()) {
    result += std::to_string(value[0]*65536*256 + value[1]*65536 +
                             value[2]*256 + value[3]);
  } else if (find(kStringTypeTerminalParameters.begin(),
                  kStringTypeTerminalParameters.end(), id) !=
                 kStringTypeTerminalParameters.end()) {
    result.insert(result.end(), value.begin(), value.end());
  } else {
    return std::string{};
  }
  return result;
}

// 显示终端参数.
void PrintTerminalParameter(ProtocolParameter const& para) {
  std::string str;
  if (!para.terminal_parameter_ids.empty()) {
    for (auto const& id : para.terminal_parameter_ids) {
      auto const& it = para.parse.terminal_parameters.find(id);
      if (it !=  para.parse.terminal_parameters.end()) {
        str = GeneralTerminalParameterFormatString(id, it->second);
        if (!str.empty()) printf("%s\n", str.c_str());
      }
    }
  } else {
    for (auto const& item : para.parse.terminal_parameters) {
      str = GeneralTerminalParameterFormatString(item.first, item.second);
        if (!str.empty()) printf("%s\n", str.c_str());
    }
  }
}

}  // namespace

// 部分参数初始化.
void JT808Server::Init(void) {
  ip_ = std::move("127.0.0.1");
  port_ = 8888;
  // 最大socket连接.
  max_connection_num_ = 10;
  // 初始化命令解析器和命令封装器.
  JT808FrameParserInit(&parser_);
  JT808FramePackagerInit(&packager_);
  // 线程运行状态初始化.
  waiting_is_running_.store(false);
  service_is_running_.store(false);
}

// 创建一个套接字, 并绑定到指定IP和端口上.
int JT808Server::InitServer(void) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<uint16_t>(port_));
#if defined(__linux__)
  addr.sin_addr.s_addr = inet_addr(ip_.c_str());
  listen_= socket(AF_INET, SOCK_STREAM, 0);
  if (listen_ == -1) {
    printf("%s[%d]: Create socket failed!!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
#elif defined(_WIN32)
  WSADATA ws_data;
  if (WSAStartup(MAKEWORD(2,2), &ws_data) != 0) {
    return -1;
  }
  listen_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listen_ == INVALID_SOCKET) {
    printf("%s[%d]: Create socket failed!!!\n", __FUNCTION__, __LINE__);
    WSACleanup();
    return -1;
  }
  addr.sin_addr.S_un.S_addr = inet_addr(ip_.c_str());
#endif
  if (Bind(listen_, reinterpret_cast<struct sockaddr *>(&addr),
              sizeof(addr)) == -1) {
    printf("%s[%d]: Connect to remote server failed!!!\n",
           __FUNCTION__, __LINE__);
    Close(listen_);
#if defined(_WIN32)
    WSACleanup();
#endif
    return -1;
  }
  is_ready_.store(true);
  return 0;
}

// 开启等待客户端连接和与客户端通信线程.
void JT808Server::Run(void) {
  if (!is_ready_) return;
  service_thread_ = std::thread(&JT808Server::ServiceHandler, this);
  service_thread_.detach();
  waiting_thread_ = std::thread(&JT808Server::WaitHandler, this);
  waiting_thread_.detach();
}

// 停止服务线程, 关闭连接并清空套接字.
void JT808Server::Stop(void) {
  if (listen_ > 0) {
    service_is_running_.store(false);
    waiting_is_running_.store(false);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    for (auto& socket : clients_) {
      Close(socket.first);
    }
    clients_.erase(clients_.begin(), clients_.end());
    Close(listen_);
    listen_ = 0;
#if defined(_WIN32)
    WSACleanup();
#endif
    is_ready_.store(false);
  }
}

// 根据提供的消息ID以及调用前此函数前对参数的设定, 生成对应的JT808格式消息,
// 并通过socket发送到服务端.
int JT808Server::PackagingAndSendMessage(
    decltype(socket(0, 0, 0)) const& socket,
    uint32_t const& msg_id,
    ProtocolParameter* para) {
  std::vector<uint8_t> msg;
  para->msg_head.msg_id = msg_id;  // 设置消息ID.
  if (JT808FramePackage(packager_, *para, &msg) < 0) {
    printf("%s[%d]: Package message failed !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  ++para->msg_head.msg_flow_num;  // 每正确生成一条命令, 消息流水号增加1.
  if (Send(socket, reinterpret_cast<char*>(msg.data()), msg.size(), 0) <= 0) {
    printf("%s[%d]: Send message failed !!!\n", __FUNCTION__, __LINE__);
    return -2;
  }
  return 0;
}

// 阻塞地从socket连接中接收一次数据, 然后按照JT808协议进行解析.
int JT808Server::ReceiveAndParseMessage(
    decltype(socket(0, 0, 0)) const& socket,
    int const& timeout,
    ProtocolParameter* para) {
  std::vector<uint8_t> msg;
  int ret = -1;
  int timeout_ms = timeout*1000;  // 超时时间, ms.
  auto tp = std::chrono::steady_clock::now();
  std::unique_ptr<char[]> buffer(
      new char[4096], std::default_delete<char[]>());
  while (1) {
    if ((ret = Recv(socket, buffer.get(), 4096, 0)) > 0) {
      msg.assign(buffer.get(), buffer.get()+ret);
      break;
    } else if (ret == 0) {
      printf("%s[%d]: Disconnect !!!\n", __FUNCTION__, __LINE__);
      return -2;
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
  if (msg.empty()) return -2;
  // 解析消息.
  if (JT808FrameParse(parser_, msg, para) == -1) {
    printf("%s[%d]: Parse message failed !!!\n", __FUNCTION__, __LINE__);
    return -1;
  }
  return 0;
}

// 等待客户端连接线程处理函数.
// 若有客户端进行连接, 先进行注册鉴权操作, 认证成功后
// 将转到主服务线程中进行数据交换.
void JT808Server::WaitHandler(void) {
  waiting_is_running_.store(true);
  if (Listen(listen_, max_connection_num_) < 0) {
    waiting_is_running_.store(false);
    Stop();
    return;
  }
  struct sockaddr_in addr;
  int len = sizeof(addr);
  while(waiting_is_running_) {
    auto socket = Accept(listen_,
        reinterpret_cast<struct sockaddr *>(&addr), &len);
    if (socket <= 0) {
      printf("%s[%d]: Invalid socket!!!\n", __FUNCTION__, __LINE__);
      break;
    }
    ProtocolParameter para{};
    if (ReceiveAndParseMessage(socket, 3, &para) < 0 ||
        para.parse.msg_head.msg_id != kTerminalRegister) {
      Close(socket);
      continue;
    }
    // 生成鉴权码.
    srand(time(NULL));
    std::string tmp(std::to_string(rand()));
    para.authentication_code.assign(tmp.begin(), tmp.end());
    para.respone_result = kRegisterSuccess;
    if (PackagingAndSendMessage(socket, kTerminalRegisterResponse, &para) < 0) {
      Close(socket);
      continue;
    }
    // 等待返回鉴权码.
    if (ReceiveAndParseMessage(socket, 3, &para) < 0) {
      Close(socket);
      continue;
    }
    // 解析返回消息并对比鉴权码.
    if (para.parse.msg_head.msg_id != kTerminalAuthentication ||
        para.authentication_code != para.parse.authentication_code) {
      Close(socket);
      continue;
    }
    para.respone_result = kSuccess;
    if (PackagingAndSendMessage(socket, kPlatformGeneralResponse, &para) < 0) {
      Close(socket);
      continue;
    }
    // printf("Connected\n");
    // 设置非阻塞模式.
    #if defined(__linux__)
      int flags = fcntl(socket, F_GETFL, 0);
      fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    #elif defined(_WIN32)
      unsigned long ul = 1;
      if (ioctlsocket(socket, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR) {
        printf("%s[%d]: Set socket nonblock failed!!!\n", __FUNCTION__, __LINE__);
        Close(socket);
        continue;
      }
    #endif
    clients_.insert(std::make_pair(socket, para));
  }
  waiting_is_running_.store(false);
  Stop();
}

// 主服务线程, 监听已连接的客户端线程处理函数.
// 暂时支持位置上报信息显示和查询终端参数应答的内容进行显示.
// 对所有非应答类命令暂时都以平台通用应答进行回应, 应答结果均为0.
// 客户端连接断开时移除相关的套接字和终端参数.
void JT808Server::ServiceHandler(void) {
  service_is_running_.store(true);
  int ret = -1;
  bool alive = false;
  std::unique_ptr<char[]> buffer(
    new char[4096], std::default_delete<char[]>());
  std::vector<uint8_t> msg;
  std::vector<uint16_t> response_cmd = {kResponseCommand,
      kResponseCommand+sizeof(kResponseCommand)/sizeof(kResponseCommand[0])};
  while(service_is_running_) {
    for (auto& socket : clients_) {
      if ((ret = Recv(socket.first, buffer.get(), 4096, 0)) > 0) {
        if (!alive) alive = true;
        msg.assign(buffer.get(), buffer.get() + ret);
        // for (auto const& uch: msg) printf("%02X ", uch);
        // printf("\n");
        if (JT808FrameParse(parser_, msg, &socket.second) == 0) {
          socket.second.respone_result = kSuccess;
          auto const& msg_id = socket.second.parse.msg_head.msg_id;
          if (msg_id == kLocationReport) {
            PrintLocationReportInfo(socket.second);
          } else if (msg_id == kGetTerminalParametersResponse) {
            PrintTerminalParameter(socket.second);
          }
          // 对于非应答类命令默认使用平台通用应答.
          if (find(response_cmd.begin(), response_cmd.end(), msg_id) ==
                  response_cmd.end()) {
            if (PackagingAndSendMessage(socket.first,
                    kPlatformGeneralResponse, &socket.second) < 0) {
              printf("%s[%d]: Disconnect !!!\n", __FUNCTION__, __LINE__);
              Close(socket.first);
              clients_.erase(socket.first);
              break;  // 删除连接时不再继续遍历, 而是重新开始遍历.
            }
          }
        }
        continue;
      } else if (ret <= 0) {
        if (ret < 0) {
#if defined(__linux__)
          if (errno == EINTR || errno == EAGAIN ||
              errno == EWOULDBLOCK) {
            continue;
          }
#elif defined(_WIN32)
          auto wsa_errno = WSAGetLastError();
          if (wsa_errno == WSAEINTR || wsa_errno == WSAEWOULDBLOCK) continue;
#endif
        }
        printf("%s[%d]: Disconnect !!!\n", __FUNCTION__, __LINE__);
        Close(socket.first);
        clients_.erase(socket.first);
        if (!alive) alive = true;
        break;  // 删除连接时不再继续遍历, 而是重新开始遍历.
      }
    }
    if (!alive) {
      std::this_thread::sleep_for(std::chrono:: milliseconds(10));
    }
    alive = false;
  }
  service_is_running_.store(false);
  Stop();
}

}  // namespace libjt808
