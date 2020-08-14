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

// @File    :  jt808_server.cc
// @Version :  1.0
// @Time    :  2020/06/24 09:57:58
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <iostream>
#include <thread>

#include "jt808/server.h"


namespace {

constexpr uint8_t kManufacturerId[] = {
  'J', 'T', '8', '0', '8'
};

}  // namespace

int main(int argc, char **argv) {
  libjt808::JT808Server server;
  server.Init();
  server.SetServerAccessPoint("127.0.0.1", 8888);
  if (server.InitServer() == 0) {
    server.Run();
    std::string cmd;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (server.service_is_running()) {
      // std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cin >> cmd;
      printf("cmd: %s\n", cmd.c_str());
      if (cmd == "upgrade") {
        std::vector<uint8_t> m_id(
            kManufacturerId, kManufacturerId + sizeof(kManufacturerId));
        server.UpgradeRequestByPhoneNumber(
            "13395279527", 52, m_id, "1.0.0", "./upgrade_send.bin");
      }
    }
    server.Stop();
  }
  return 0;
}
