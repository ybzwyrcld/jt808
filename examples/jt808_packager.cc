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

// @File    :  jt808_packager.cc
// @Version :  1.0
// @Time    :  2020/06/28 11:07:11
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include <stdio.h>

#include <iostream>

#include "jt808/packager.h"


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

void RegistePackage(libjt808::Packager const&packager) {
  libjt808::ProtocolParameter parameter {};
  std::vector<uint8_t> raw_msg;
  parameter.msg_head.msg_id = 0x0100;
  parameter.msg_head.phone_num = "13523339527";
  parameter.msg_head.msg_flow_num = 1;
  parameter.register_info.province_id = 0x002c;
  parameter.register_info.city_id = 0x012c;
  parameter.register_info.manufacturer_id.assign(
      kManufacturerId, kManufacturerId+sizeof(kManufacturerId));
  parameter.register_info.terminal_model.assign(
      kTerminalModel, kTerminalModel+sizeof(kTerminalModel));
  parameter.register_info.terminal_id.assign(
      kTerminalId, kTerminalId+sizeof(kTerminalId));
  parameter.register_info.car_plate_color = libjt808::VehiclePlateColor::kBlue;
  parameter.register_info.car_plate_num = "粤B99999";
  libjt808::JT808FramePackage(packager, parameter, &raw_msg);
  printf("raw msg: ");
  for (auto& uch : raw_msg) printf("%02X ", uch);
  printf("\n");
  return;
}

void LocaltionReportPackage(libjt808::Packager const&packager) {
  libjt808::ProtocolParameter parameter {};
  std::vector<uint8_t> raw_msg;
  parameter.msg_head.msg_id = 0x0200;
  parameter.msg_head.phone_num = "13523339527";
  parameter.msg_head.msg_flow_num = 1;
  parameter.location_info.status.bit.gps_en = 1;
  parameter.location_info.status.bit.beidou_en = 1;
  parameter.location_info.status.bit.positioning = 1;
  printf("%04X\n", parameter.location_info.status.value);
  parameter.location_info.latitude = 22570336;
  parameter.location_info.longitude = 113937577;
  parameter.location_info.altitude = 54;
  parameter.location_info.speed = 8;
  parameter.location_info.bearing = 0;
  parameter.location_info.time = "200702145429";
  parameter.location_extension.insert(std::pair<uint8_t,
      std::vector<uint8_t>>(0x31, std::vector<uint8_t>{11}));
  parameter.location_extension.insert(
      std::pair<uint8_t, std::vector<uint8_t>>(0xE0, std::vector<uint8_t>{1}));
  parameter.location_extension.insert(
      std::pair<uint8_t, std::vector<uint8_t>>(0xEE, std::vector<uint8_t>{2}));
  if (libjt808::JT808FramePackage(packager, parameter, &raw_msg) == 0) {
    printf("raw msg: ");
    for (auto& uch : raw_msg) printf("0x%02X, ", uch);
    printf("\n");
  }

  return;
}

}  // namespace

int main(int argc, char **argv) {
  libjt808::Packager jt808_packager;
  libjt808::JT808FramePackagerInit(&jt808_packager);
  RegistePackage(jt808_packager);
  LocaltionReportPackage(jt808_packager);
  return 0;
}
