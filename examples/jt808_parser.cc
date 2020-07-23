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

// @File    :  jt808_parser.cc
// @Version :  1.0
// @Time    :  2020/06/24 13:59:10
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <stdio.h>

#include <iostream>

#include "jt808/parser.h"


using std::cin;
using std::cout;

namespace {

constexpr uint8_t kSeviveGeneralResponse[] = {
  0x7E, 0x80, 0x01, 0x00, 0x05,
  0x01, 0x38, 0x26, 0x53, 0x98, 0x49,
  0x00, 0x02,
  0x00, 0x02, 0x01, 0x02, 0x00, 0x1A, 0x7E
};

constexpr uint8_t kLocationReport[] = {
  0x7E, 0x02, 0x00, 0x00, 0x26,
  0x01, 0x35, 0x23, 0x33, 0x95, 0x27,
  0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x02,
  0x01, 0x58, 0x65, 0x60,
  0x06, 0xCA, 0x8C, 0xA9,
  0x00, 0x36,
  0x00, 0x08,
  0x00, 0x00,
  0x20, 0x07, 0x02, 0x14, 0x54, 0x29,
  0x31, 0x01, 0x0B,
  0xE0, 0x03,
  0xEE, 0x01, 0x02,
  0x4F, 0x7E
};

void PlatformGeneralResponseTest(void) {
  libjt808::Parser jt808_parser;
  libjt808::ProtocolParameter parameter {};
  libjt808::JT808FrameParserInit(&jt808_parser);
  std::vector<uint8_t> raw_msg;
  raw_msg.assign(kSeviveGeneralResponse,
                 kSeviveGeneralResponse+sizeof(kSeviveGeneralResponse));
  libjt808::JT808FrameParse(jt808_parser, raw_msg, &parameter);
  printf("response: id=%04X, flow_num=%d, result=%d\n",
         parameter.respone_msg_id,
         parameter.respone_flow_num,
         parameter.respone_result);
}

void PlatformLocationReportParseTest(void) {
  libjt808::Parser jt808_parser;
  libjt808::ProtocolParameter parameter {};
  libjt808::JT808FrameParserInit(&jt808_parser);
  std::vector<uint8_t> raw_msg;
  raw_msg.assign(kLocationReport,
                 kLocationReport+sizeof(kLocationReport));
  if (libjt808::JT808FrameParse(jt808_parser, raw_msg, &parameter) == 0) {
    auto const& basic_info = parameter.parse.location_info;
    auto const& extension_info = parameter.parse.location_extension;
    printf("psoition status: %d\n", basic_info.status.bit.positioning);
    printf("latitude: %.6lf\n", basic_info.latitude/1e6);
    printf("longitude: %.6lf\n", basic_info.longitude/1e6);
    printf("atitude: %d\n", basic_info.altitude);
    printf("speed: %d\n", basic_info.speed*10);
    printf("bearing: %d\n", basic_info.bearing);
    printf("time: %s\n", basic_info.time.c_str());
    auto it = extension_info.find(0x31);
    if (it != extension_info.end())
      printf("satellites number: %d\n", it->second[0]);
    it = extension_info.find(0xE0);
    if (it != extension_info.end())
      printf("has extension custom\n");
    it = extension_info.find(0xEE);
    if (it != extension_info.end())
      printf("position fix status: %d\n", it->second[0]);
  }
}

}  // namespace


int main(int argc, char **argv) {
  PlatformGeneralResponseTest();
  PlatformLocationReportParseTest();
  return 0;
}
