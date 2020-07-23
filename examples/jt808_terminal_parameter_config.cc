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

// @File    :  jt808_terminal_parameter_config.cc
// @Version :  1.0
// @Time    :  2020/07/16 17:53:18
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <stdio.h>
#include <math.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "jt808/packager.h"
#include "jt808/parser.h"
#include "jt808/terminal_parameter.h"


using std::cin;
using std::cout;

int main(int argc, char **argv) {
  //
  // 808初始化.
  //
  // 协议参数
  libjt808::ProtocolParameter svr_para {};
  libjt808::ProtocolParameter cli_para {};
  cli_para.msg_head.phone_num = std::move(std::string("13523339527"));
  cli_para.msg_head.msg_flow_num = 1;
  svr_para.msg_head.phone_num = std::move(std::string("13523339527"));
  svr_para.msg_head.msg_flow_num = 1;
  // 命令生成器初始化.
  libjt808::Packager jt808_packager;
  libjt808::JT808FramePackagerInit(&jt808_packager);
  // 命令解析器初始化.
  libjt808::Parser jt808_parser;
  libjt808::JT808FrameParserInit(&jt808_parser);
  std::vector<uint8_t> out;
  // 预设几个终端参数.
  libjt808::PackagingTerminalParameterNtripCors(
      "192.168.3.111", 8002, "user01", "123456", "RTCM23_GPS", 10,
      &svr_para.terminal_parameters);
  std::string ip;
  uint16_t port;
  std::string usr;
  std::string pwd;
  std::string mp;
  uint8_t intv = 0;
  // 输出配置的终端参数.
  if (libjt808::ParseTerminalParameterNtripCors(
      svr_para.terminal_parameters,
      &ip, &port, &usr, &pwd, &mp, &intv) == 0) {
    cout << "Set para: " <<
            ip << ", " <<
            port << ", " <<
            usr << ", " <<
            pwd << ", "
            << mp << ", " <<
            std::to_string(intv) << "\n";
  }
  // 平台生成设置终端参数消息.
  svr_para.msg_head.msg_id = libjt808::kSetTerminalParameters;
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++svr_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 终端解析设置终端参数消息.
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  // 拷贝终端参数.
  for (auto const& it : cli_para.parse.terminal_parameters) {
    cli_para.terminal_parameters.insert(it);
  }
  // 终端生成通用应答.
  cli_para.msg_head.msg_id = libjt808::kTerminalGeneralResponse;
  cli_para.respone_result =libjt808::kSuccess;
  if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++cli_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 平台生成查询终端参数消息.
  svr_para.msg_head.msg_id = libjt808::kGetTerminalParameters;
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++svr_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 终端解析查询终端参数消息.
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  // 终端生成查询终端参数应答消息.
  cli_para.msg_head.msg_id = libjt808::kGetTerminalParametersResponse;
  if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++cli_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 平台解析查询终端参数应答.
  if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  // 输出解析的终端参数.
  if (libjt808::ParseTerminalParameterNtripCors(
      svr_para.parse.terminal_parameters,
      &ip, &port, &usr, &pwd, &mp, &intv) == 0) {
    cout << "Get all para: " <<
            ip << ", " <<
            port << ", " <<
            usr << ", " <<
            pwd << ", " << mp
            << ", " <<
            std::to_string(intv) << "\n";
  }
  // 平台生成查询指定终端参数消息.
  svr_para.terminal_parameter_ids.clear();
  svr_para.terminal_parameter_ids.push_back(libjt808::kNtripCorsIP);
  svr_para.terminal_parameter_ids.push_back(libjt808::kNtripCorsPort);
  svr_para.terminal_parameter_ids.push_back(libjt808::kNtripCorsUser);
  svr_para.terminal_parameter_ids.push_back(libjt808::kNtripCorsPasswd);
  svr_para.terminal_parameter_ids.push_back(libjt808::kNtripCorsMountPoint);
  svr_para.terminal_parameter_ids.push_back(
      libjt808::kNtripCorsGGAReportInterval);
  svr_para.msg_head.msg_id = libjt808::kGetSpecificTerminalParameters;
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++svr_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 终端解析查询终端参数消息.
  cli_para.parse.terminal_parameter_ids.clear();
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  // 拷贝查询终端参数ID.
  cli_para.terminal_parameter_ids.clear();
  for (auto const& it : cli_para.parse.terminal_parameter_ids) {
    cli_para.terminal_parameter_ids.push_back(it);
  }
  // 终端生成查询终端参数应答消息.
  cli_para.msg_head.msg_id = libjt808::kGetTerminalParametersResponse;
  if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++cli_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 平台解析查询终端参数应答.
  svr_para.parse.terminal_parameters.clear();
  if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  // 输出解析的终端参数.
  if (libjt808::ParseTerminalParameterNtripCors(
      svr_para.parse.terminal_parameters,
      &ip, &port, &usr, &pwd, &mp, &intv) == 0) {
    cout << "Get special para: " <<
            ip << ", " <<
            port << ", " <<
            usr << ", " <<
            pwd << ", " <<
            mp << ", " <<
            std::to_string(intv) << "\n";
  }
  return 0;
}
