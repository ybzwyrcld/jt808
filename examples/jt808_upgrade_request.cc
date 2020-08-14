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

// @File    :  jt808_upgrade_request.cc
// @Version :  1.0
// @Time    :  2020/07/14 09:17:58
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "jt808/packager.h"
#include "jt808/parser.h"

namespace {

constexpr uint8_t kManufacturerId[] = {
  'S', 'K', 'O', 'E', 'M'
};

void print_usage(char const* program) {
  printf("Usage: %s upgrade_file_path\n", program);
}

}  // namespace

int main(int argc, char **argv) {
  // 检查参数.
  if (argc != 2) {
    print_usage(argv[0]);
    return -1;
  }
  // 读取升级文件内容.
  std::ifstream ifs;
  ifs.open(argv[1], std::ios::in|std::ios::binary);
  if (!ifs.is_open()) {
    printf("Open file failed!!!\n");
    return false;
  }
  ifs.seekg(0, std::ios::end);
  size_t length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  std::unique_ptr<char[]> buffer(
    new char[length], std::default_delete<char[]>());
  ifs.read(buffer.get(), length);
  ifs.close();
  printf("Read %u size buffer\n", static_cast<uint32_t>(length));
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
  //
  // 仅生成一包升级数据包.
  //
  // GNSS模块升级.
  svr_para.upgrade_info.upgrade_type = libjt808::kGNSS;
  // 制造商ID.
  svr_para.upgrade_info.manufacturer_id.clear();
  svr_para.upgrade_info.manufacturer_id.assign(
      kManufacturerId, kManufacturerId + sizeof(kManufacturerId));
  // 升级版本号.
  svr_para.upgrade_info.version_id.clear();
  svr_para.upgrade_info.version_id = std::move(std::string("1.0.1"));
  // 升级数据包.
  svr_para.upgrade_info.upgrade_data.clear();
  // 一条消息内容最大1023字节(转义前).
  uint16_t max_content = 1023-9-svr_para.upgrade_info.version_id.size();
  std::vector<uint8_t> out;
  std::map<uint16_t, std::vector<uint8_t>> packet_msg;
  std::map<uint16_t, std::vector<uint8_t>> packet_data;
  if (length > max_content) {  // 需要分包处理.
    auto const& cli_parse_upgrade_info = cli_para.parse.upgrade_info;
    svr_para.msg_head.msgbody_attr.bit.packet = 1;  // 进行分包.
    svr_para.msg_head.total_packet =
        static_cast<uint16_t>(ceil(length*1.0/max_content));
    svr_para.msg_head.packet_seq = 1;
    uint16_t len = 0;
    for (size_t i = 0; i < length; i += max_content) {
      len = length-i;
      if (len > max_content) len = max_content;
      svr_para.upgrade_info.upgrade_data.clear();
      svr_para.upgrade_info.upgrade_data.assign(
          buffer.get()+i, buffer.get()+i+len);
      // svr_para.msg_head.msgbody_attr.bit.packet = 1;  // 进行分包.
      // 平台生成下发终端升级包消息.
      svr_para.msg_head.msg_id = libjt808::kTerminalUpgrade;
      if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
        printf("Generate message failed\n");
        return -1;
      }
      packet_msg.insert({svr_para.msg_head.packet_seq, out});
      ++svr_para.msg_head.packet_seq;
      ++svr_para.msg_head.msg_flow_num;
      printf("Raw message(except msg body): ");
      std::for_each(out.begin(), out.begin()+17, [] (uint8_t const& uch) {
        printf("%02X ", uch);
      });
      printf("%02X %02X\n", *(out.end()-2), *(out.end()-1));
      // 这里制造一包丢失数据, 测试补传分包请求功能.
      if (svr_para.msg_head.packet_seq == 3) continue;
      // 终端解析下发终端升级包消息.
      if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
        printf("Parse message failed\n");
        return -1;
      }
      // 预先保存第一包流水号, 在需要补传分包时用到.
      if ((cli_para.parse.msg_head.msgbody_attr.bit.packet == 1) &&
          (cli_para.parse.msg_head.packet_seq == 1)) {
        cli_para.fill_packet.first_packet_msg_flow_num =
            cli_para.parse.msg_head.msg_flow_num;
      }
      // 终端接收到下发升级包, 使用终端通用应答回应.
      cli_para.msg_head.msg_id = libjt808::kTerminalGeneralResponse;
      cli_para.respone_result =libjt808::kSuccess;
      if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
        printf("Generate message failed\n");
        return -1;
      }
      ++cli_para.msg_head.msg_flow_num;
      packet_data.insert({cli_para.parse.msg_head.packet_seq,
                          cli_parse_upgrade_info.upgrade_data});
      // 平台解析终端通用应答.
       if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para) < 0) {
        printf("Parse message failed\n");
        return -1;
      }
      if (svr_para.parse.respone_result != libjt808::kSuccess) {
        continue;
      }
    }
    svr_para.msg_head.msgbody_attr.bit.packet = 0;
    // 补传分包检查.
    if (packet_data.size() != cli_para.parse.msg_head.total_packet) {
      // 检查丢失的子包序号.
      cli_para.fill_packet.packet_id.clear();
      for (uint16_t i = 1; i <= cli_para.parse.msg_head.total_packet; ++i) {
        auto it = packet_data.find(i);
        if (it == packet_data.end()) {
          cli_para.fill_packet.packet_id.push_back(i);
        }
      }
      // 终端生成补传分包请求消息.
      cli_para.msg_head.msg_id = libjt808::kFillPacketRequest;
      if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
        printf("Generate message failed\n");
        return -1;
      }
      ++cli_para.msg_head.msg_flow_num;
      // printf("Raw message: ");
      // for (auto const& uch : out) printf("%02X ", uch);
      // printf("\n");
      // 平台解析补传分包请求消息.
      if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para) < 0) {
        printf("Parse message failed\n");
        return -1;
      }
      // 平台重新发送丢失的子包, 直接发送原始的分包消息, 不用重新打包.
      out.clear();
      for (auto const& id : svr_para.parse.fill_packet.packet_id) {
        auto it = packet_msg.find(id);
        if (it == packet_msg.end()) continue;
        out.clear();
        out.assign(it->second.begin(), it->second.end());
        // 终端解析下发终端升级包消息.
        if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
          printf("Parse message failed\n");
          return -1;
        }
        // 终端接收到下发升级包, 使用终端通用应答回应.
        cli_para.msg_head.msg_id = libjt808::kTerminalGeneralResponse;
        cli_para.respone_result =libjt808::kSuccess;
        if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
          printf("Generate message failed\n");
          return -1;
        }
        ++cli_para.msg_head.msg_flow_num;
        packet_data.insert({cli_para.parse.msg_head.packet_seq,
                            cli_parse_upgrade_info.upgrade_data});
        // 平台解析终端通用应答.
        if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para) < 0) {
          printf("Parse message failed\n");
          return -1;
        }
        if (svr_para.parse.respone_result != libjt808::kSuccess) {
          continue;
        }
      }
    }
    printf("Receive completed\n");
    // 写入文件.
    std::ofstream ofs;
    ofs.open("./temp.txt", std::ios::out|std::ios::binary|std::ios::trunc);
    if (ofs.is_open()) {
      for (auto& data : packet_data) {
        ofs.write(reinterpret_cast<char*>(data.second.data()),
                  data.second.end()- data.second.begin());
      }
      ofs.close();
    }
  } else {  // 不用分包.
    svr_para.upgrade_info.upgrade_data.assign(
        buffer.get(), buffer.get()+length);
    // 生成升级消息.
    svr_para.msg_head.msg_id = libjt808::kTerminalUpgrade;
    if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
      printf("Generate message failed\n");
      return -1;
    }
    ++svr_para.msg_head.msg_flow_num;
    printf("Raw message(except msg body): ");
    std::for_each(out.begin(), out.begin()+13, [] (uint8_t const& uch) {
      printf("%02X ", uch);
    });
    printf("%02X %02X\n", *(out.end()-2), *(out.end()-1));
    // 终端解析升级请求.
    if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
      printf("Parse message failed\n");
      return -1;
    }
    // 写入文件.
    std::ofstream ofs;
    ofs.open("./temp.txt", std::ios::out|std::ios::binary|std::ios::trunc);
    if (ofs.is_open()) {
      auto& data = cli_para.parse.upgrade_info.upgrade_data;
      ofs.write(reinterpret_cast<char*>(data.data()),
                data.end()- data.begin());
      ofs.close();
    }
  }
  // 终端回应升级结果.
  cli_para.upgrade_info.upgrade_type =
      cli_para.parse.upgrade_info.upgrade_type;
  cli_para.upgrade_info.upgrade_result = libjt808::kTerminalUpgradeSuccess;
  cli_para.msg_head.msg_id = libjt808::kTerminalUpgradeResultReport;
  if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++cli_para.msg_head.msg_flow_num;
  // printf("Raw message: ");
  // for (auto const& uch : out) printf("%02X ", uch);
  // printf("\n");
  // 平台解析升级结果.
  if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  auto const& svr_parse_upgrade_info = svr_para.parse.upgrade_info;
  printf("Upgrade result:\n");
  printf("  upgrade type: %d\n",  svr_parse_upgrade_info.upgrade_type);
  printf("  upgrade result: %d\n", svr_parse_upgrade_info.upgrade_result);
  return 0;
}
