// MIT License
//
// Copyright (c) 2021 Yuming Meng
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

// @File    :  jt808_multimedia_upload_client.cc
// @Version :  1.0
// @Time    :  2021/05/11 14:26:20
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "jt808/client.h"


int main(int argc, char **argv) {
  libjt808::JT808Client client;
  client.Init();
  client.SetRemoteAccessPoint("127.0.0.1", 8888);
  client.SetTerminalPhoneNumber("13395279527");
  if ((client.ConnectRemote() == 0) &&
      (client.JT808ConnectionAuthentication() == 0)) {
    client.Run();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string cmd;
    while (client.service_is_running()) {
      std::cin >> cmd;
      if (cmd == "upload") {
        client.MultimediaUpload("./test.bin", {0});
      }
    }
    client.Stop();
  }
  return 0;
}