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

// @File    :  jt808_client.cc
// @Version :  1.0
// @Time    :  2020/07/17 10:27:07
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <iostream>
#include <thread>

#include "jt808/client.h"


int main(int argc, char **argv) {
  libjt808::JT808Client client;
  client.Init();
  client.SetRemoteAccessPoint("127.0.0.1", 8888);
  if ((client.ConnectRemote() == 0) &&
      (client.JT808ConnectionAuthentication() == 0)) {
    client.UpdateLocation(22.570336, 113.937577, 54.0, 60, 0, "200702145429");
    libjt808::StatusBit status_bit {};
    status_bit.bit.positioning = 1;  // 已成功定位.
    client.SetStatusBit(status_bit.value);
    client.UpdateGNSSSatelliteNumber(11);
    // 自定义位置信息附加项.
    // client.UpdateGNSSPositioningSolutionStatus(2);  // 伪距差分解.
    client.SetJT808Service("127.0.0.1", 8888, "13595279527", 10);
    client.Run();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (client.service_is_running()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    client.Stop();
  }
  return 0;
}
