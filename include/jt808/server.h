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

// @File    :  server.h
// @Version :  1.0
// @Time    :  2020/06/24 09:59:07
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_SERVER_H_
#define JT808_SERVER_H_

#if defined(__linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <map>

#include "packager.h"
#include "parser.h"
#include "protocol_parameter.h"
#include "terminal_parameter.h"


namespace libjt808 {

// JT808平台.
// 已实现了终端注册, 终端鉴权, 心跳包, 位置信息汇报功能.
// 鉴权成功后对所有命令暂时以平台通用应答作为消息回应.
//
// Example:
//     JT808Server server;
//     server.Init();
//     server.SetServerAccessPoint("127.0.0.1", 8888);
//     if ((server.InitServer() == 0)) {
//       server.Run();
//       std::this_thread::sleep_for(std::chrono::seconds(1));
//       while (server.service_is_running()) {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//       }
//       server.Stop();
//     }
class JT808Server {
 public:
  JT808Server() {}
  ~JT808Server() {}
  // 参数初始化.
  void Init(void);

  // 设置服务端地址.
  void SetServerAccessPoint(std::string const&ip, int const& port) {
    ip_ = ip;
    port_ = port;
  }
  // 初始化服务端.
  int InitServer(void);

  //
  // 服务线程运行与终止.
  //
  // 启动服务线程.
  void Run(void);
  // 停止服务线程.
  void Stop(void);
  // 获取当前服务线程运行状态.
  bool service_is_running(void) const {
    return service_is_running_;
  }

  //
  //  外部获取和设置当前的通用消息体解析和封装函数, 用于重写或新增命令支持.
  //  必须在调用Init()成员函数后才可以使用.
  //
  // 获取通用JT808协议封装器.
  Packager& packager(void) { return packager_; }
  Packager const& packager(void) const { return packager_; }
  void packager(Packager* packager) const {
    if (packager == nullptr) return;
    *packager = packager_;
  }
  // 设置通用JT808协议封装器.
  void set_packager(Packager const& packager) { packager_ = packager; }
  // 获取通用JT808协议解析器.
  Parser& parser(void) { return parser_; }
  Parser const& parser(void) const {  return parser_; }
  void parser(Parser* parser) const {
    if (parser == nullptr) return;
    *parser = parser_;
  }
  // 设置通用JT808协议解析器.
  void set_parser(Parser const& parser) { parser_ = parser; }

  // 升级请求.
  // Args:
  //     socket:  客户端的socket.
  //     upgrade_type: 升级类型.
  //     path:  升级文件路径.
  // Returns:
  //     成功返回0, 失败返回-1.
  int UpgradeRequest(decltype(socket(0, 0, 0)) const& socket,
                     int const& upgrade_type,
                     std::vector<uint8_t> const& manufacturer_id,
                     std::string const& version_id,
                     char const* path);
  
  // 升级请求.
  // Args:
  //     phone:  客户端的终端手机号.
  //     upgrade_type: 升级类型.
  //     path:  升级文件路径.
  // Returns:
  //     成功返回0, 失败返回-1.
  int UpgradeRequestByPhoneNumber(std::string const& phone,
                                  int const& upgrade_type,
                                  std::vector<uint8_t> const& manufacturer_id,
                                  std::string const& version_id,
                                  char const* path) {
    for (auto const& item : clients_) {
      if (item.second.msg_head.phone_num == phone) {
        return UpgradeRequest(item.first, upgrade_type,
                              manufacturer_id, version_id, path);
      }
    }
    return -1;
  }
  // 
  // 多媒体数据上传.
  //
  using MultimediaDataUploadCallback =
      std::function<void (MultiMediaDataUpload const&)>;
  void OnMultimediaDataUploaded(MultimediaDataUploadCallback const& callback) {
    multimedia_data_upload_callback_ = callback;
  }

  // 通用消息封装和发送函数.
  // Args:
  //     socket:  客户端的socket.
  //     msg_id:  消息ID.
  //     para: 协议参数.
  // Returns:
  //     成功返回0, 失败返回-1.
  int PackagingAndSendMessage(decltype(socket(0, 0, 0)) const& socket,
                              uint32_t const& msg_id,
                              ProtocolParameter* para);

  // 通用消息接收和解析函数.
  // 阻塞函数.
  // 鉴权通过的客户端禁止调用.
  // Args:
  //     client:  客户端的socket.
  //     timeout:  超时时间, 单位秒(s).
  //     para: 协议参数.
  // Returns:
  //     成功返回0, 失败返回-1.
  int ReceiveAndParseMessage(decltype(socket(0, 0, 0)) const& socket,
                             int const& timeout,
                             ProtocolParameter* para);

 private:
  // 等待客户端连接线程处理函数.
  void WaitHandler(void);
  // 主服务线程处理函数.
  void ServiceHandler(void);

  decltype(socket(0, 0, 0)) listen_;  // 监听的socket.
  std::atomic_bool is_ready_;  // 服务端socket状态.
  std::string ip_;  // 服务端IP地址.
  int port_;  // 服务端端口.
  int max_connection_num_;
  MultimediaDataUploadCallback multimedia_data_upload_callback_;
  std::thread waiting_thread_;  // 等待客户端连接线程.
  std::atomic_bool waiting_is_running_;  // 等待客户端连接线程运行标志.
  std::thread service_thread_;  // 主服务线程.
  std::atomic_bool service_is_running_;  // 主服务线程运行标志.
  Packager packager_;  // 通用JT808协议封装器.
  Parser parser_;  // 通用JT808协议解析器.
  // 客户端的socket(key)-客户端的协议参数(value).
  std::map<decltype(socket(0, 0, 0)), ProtocolParameter> clients_;
  // 处于升级状态的客户端连接.
  std::map<decltype(socket(0, 0, 0)), int> is_upgrading_clients_;
};

}  // namespace libjt808

#endif  // JT808_SERVER_H_
