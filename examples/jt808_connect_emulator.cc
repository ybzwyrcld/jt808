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

// @File    :  jt808_connect_emulator.cc
// @Version :  1.0
// @Time    :  2020/06/28 17:54:50
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <cstdlib>
#include <cstdio>
#include <time.h>

#include <iostream>

#include "jt808/packager.h"
#include "jt808/parser.h"


using std::cin;
using std::cout;

namespace {

constexpr uint8_t kManufacturerId[] = {
  'S', 'K', 'O', 'E', 'M'
};

constexpr uint8_t kTerminalModel[] = {
  'S', 'K', '9', '1', '5', '1'
};

constexpr uint8_t kTerminalId[] = {
  '0', '0', '0', '0', '0', '1'
};

int ConnectEmulator(void) {
  libjt808::ProtocolParameter svr_para {};
  libjt808::ProtocolParameter cli_para {};
  //
  // 初始化.
  //
  // 终端参数初始化.
  cli_para.msg_head.phone_num = "13523339527";
  cli_para.msg_head.msg_flow_num = 1;
  cli_para.register_info.province_id = 0x002c;
  cli_para.register_info.city_id = 0x012c;
  cli_para.register_info.manufacturer_id.assign(
      kManufacturerId, kManufacturerId+sizeof(kManufacturerId));
  cli_para.register_info.terminal_model.assign(
      kTerminalModel, kTerminalModel+sizeof(kTerminalModel));
  cli_para.register_info.terminal_id.assign(
      kTerminalId, kTerminalId+sizeof(kTerminalId));
  cli_para.register_info.car_plate_color = libjt808::VehiclePlateColor::kBlue;
  cli_para.register_info.car_plate_num = "粤B99999";
  // 平台参数初始化.
  svr_para.msg_head.msg_flow_num = 1;
  // 命令生成器初始化.
  libjt808::Packager packager;
  libjt808::JT808FramePackagerInit(&packager);
  // 命令解析器初始化.
  libjt808::Parser parser;
  libjt808::JT808FrameParserInit(&parser);
  //
  // 终端生成注册消息.
  //
  cli_para.msg_head.msg_id = 0x0100;
  std::vector<uint8_t> out;
  if (libjt808::JT808FramePackage(packager, cli_para, &out) < 0) {
    printf("Generate register message failed\n");
    return -1;
  }
  cli_para.msg_head.msg_flow_num++;  // 每生成一次命令, 消息流水号增加.
  printf("Register message: ");
  for (auto& uch : out) printf("%02X ", uch);
  printf("\n");
  //
  // 平台解析注册消息.
  //
  if (libjt808::JT808FrameParse(parser, out, &svr_para) < 0) {
    printf("Parse register message failed\n");
    return -1;
  }
  // 显示解析后的注册信息.
  printf("\nRegister Info:\n");
  auto const& register_info = svr_para.parse.register_info;
  printf("  phone number: %s\n", svr_para.parse.msg_head.phone_num.c_str());
  printf("  province id: 0x%04X\n", register_info.province_id);
  printf("  city id: 0x%04X\n", register_info.city_id);
  std::string str;
  str.clear();
  str.assign(register_info.manufacturer_id.begin(),
             register_info.manufacturer_id.end());
  printf("  manufacturer id: %s\n", str.c_str());
  str.clear();
  str.assign(register_info.terminal_model.begin(),
             register_info.terminal_model.end());
  printf("  terminal model: %s\n", str.c_str());
  str.clear();
  str.assign(register_info.terminal_id.begin(),
             register_info.terminal_id.end());
  printf("  terminal id: %s\n", str.c_str());
  printf("  car plate color: %d\n", register_info.car_plate_color);
  if (register_info.car_plate_color != libjt808::VehiclePlateColor::kVin) {
    str.clear();
    str.assign(register_info.car_plate_num.begin(),
               register_info.car_plate_num.end());
    printf("  car plate number: %s\n", str.c_str());
  }
  printf("\n");
  //
  // 平台生成注册应答消息.
  //
  // 先随机生成一个鉴权码.
  srand(time(NULL));
  std::string tmp(std::to_string(rand()));
  printf("Generated authentication code: %s\n", tmp.c_str());
  svr_para.authentication_code.clear();
  for (auto& ch : tmp) svr_para.authentication_code.push_back(ch);
  svr_para.msg_head.msg_id = 0x8100;
  svr_para.respone_result = 0;  // 应答结果: 成功.
  if (libjt808::JT808FramePackage(packager, svr_para, &out) < 0) {
    printf("Generate register respone message failed\n");
    return -1;
  }
  svr_para.msg_head.msg_flow_num++;  // 每生成一次命令, 消息流水号增加.
  printf("Register respone message: ");
  for (auto& uch : out) printf("%02X ", uch);
  printf("\n");
  //
  //  终端解析注册应答消息.
  //
  cli_para.parse.respone_result = libjt808::kTerminalHaveBeenRegistered;
  if (libjt808::JT808FrameParse(parser, out, &cli_para) < 0) {
    printf("Parse register respone message failed\n");
    return -1;
  }
  if (cli_para.parse.respone_result != libjt808::kRegisterSuccess) {
    printf("Register respone result failed\n");
    return -1;
  }
  // 得到鉴权码.
  std::string authen_code;
  authen_code.assign(cli_para.parse.authentication_code.begin(),
                     cli_para.parse.authentication_code.end());
  printf("Parsed authentication code: %s\n", authen_code.c_str());
  //
  // 终端生成终端鉴权消息.
  //
  cli_para.msg_head.msg_id = 0x0102;
  if (libjt808::JT808FramePackage(packager, cli_para, &out) < 0) {
    printf("Generate authentication message failed\n");
    return -1;
  }
  cli_para.msg_head.msg_flow_num++;  // 每生成一次命令, 消息流水号增加.
  printf("Authentication message: ");
  for (auto& uch : out) printf("%02X ", uch);
  printf("\n");
  //
  // 平台解析终端鉴权消息.
  //
  if (libjt808::JT808FrameParse(parser, out, &svr_para) < 0) {
    printf("Parse register message failed\n");
    return -1;
  }
  // 对比鉴权码.
  std::string report_authen_code;
  report_authen_code.assign(svr_para.parse.authentication_code.begin(),
                            svr_para.parse.authentication_code.end());
  printf("Report authentication code: %s\n",report_authen_code.c_str());
  if (svr_para.authentication_code != svr_para.parse.authentication_code) {
    printf("Authentication failed\n");
    return -1;
  }
  printf("Comparison of authentication code success\n");
  //
  // 平台生成通用应答消息.
  //
  svr_para.msg_head.msg_id = 0x8001;
  svr_para.respone_result = 0;  // 应答结果: 成功.
  if (libjt808::JT808FramePackage(packager, svr_para, &out) < 0) {
    printf("Generate authentication respone message failed\n");
    return -1;
  }
  svr_para.msg_head.msg_flow_num++;  // 每生成一次命令, 消息流水号增加.
  printf("Authentication respone message: ");
  for (auto& uch : out) printf("%02X ", uch);
  printf("\n");
  //
  //  终端解析通用应答消息.
  //
  cli_para.parse.respone_result = libjt808::kFailure;
  if (libjt808::JT808FrameParse(parser, out, &cli_para) < 0) {
    printf("Parse register respone message failed\n");
    return -1;
  }
  if (cli_para.parse.respone_result != libjt808::kSuccess) {
    printf("Authentication respone result failed\n");
    printf("Connection failed.\n");
    return -1;
  } else {
    printf("Connected.\n");
  }
  return 0;
}

}  // namespace


int main(int argc, char **argv) {
  return ConnectEmulator();
}
