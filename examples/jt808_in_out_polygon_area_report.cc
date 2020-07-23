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

// @File    :  jt808_in_out_polygon_area_report.cc
// @Version :  1.0
// @Time    :  2020/07/07 09:01:04
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

#include "nmea_parser.h"
#include "jt808/packager.h"
#include "jt808/parser.h"


using std::cin;
using std::cout;
using libjt808::LocationPoint;

namespace {

void print_usage(char const* program) {
  printf("Usage: %s nmea_file_path\n", program);
}

int SetPolygonArea(uint32_t const& area_id,
                   libjt808::LocationPoint const& center_point,
                   libjt808::PolygonArea* area) {
  if (area == nullptr) return -1;
  area->area_id = area_id;
  area->area_attribute.bit.in_alarm_to_server = 1;
  area->area_attribute.bit.out_alarm_to_server = 1;
  area->vertices.clear();
  libjt808::LocationPoint location_point = center_point;
  // 左上.
  location_point.longitude += 1*1e-6;
  location_point.latitude += 1*1e-6;
  area->vertices.push_back(location_point);
  // 右上.
  location_point.latitude -= 2*1e-6;
  area->vertices.push_back(location_point);
  // 右下.
  location_point.longitude -= 2*1e-6;
  area->vertices.push_back(location_point);
  // 左下.
  location_point.latitude += 2*1e-6;
  area->vertices.push_back(location_point);
  return 0;
}

// 地球半径, 取赤道半径取值, 单位为千米(Km).
constexpr double kEarthRadius = 6378.137;
//  同一经度下1°纬度差的直线距离, 单位为千米(Km).
constexpr double kDistanceStep = 111.31949079327372;
constexpr double kPI = 3.1415926535897931;

constexpr double DegreeToRadian(double const& degree) {
  return (degree * kPI / 180.0);
}
constexpr double LonLatMax(double const& lval, double const& rval) {
  return (lval > rval ? lval:rval);
}

constexpr double LonLatMin(double const& lval, double const& rval) {
  return (lval < rval ? lval:rval);
}

bool LonLatEqual(double const& lval, double const& rval) {
  return (fabs(lval-rval) < 1e-7);
}

// double DistanceInMap(double const& longitudel, double const& latitudel,
//                      double const& longituder, double const& latituder) {
//   auto rad_lat1 = DegreeToRadian(latitudel);
//   auto rad_lat2 = DegreeToRadian(latituder);
//   auto d_lat = rad_lat1 - rad_lat2;
//   auto d_lon = (DegreeToRadian(longitudel)- DegreeToRadian(longituder));
//   auto distance = 2*asin(sqrt(pow(sin(d_lat/2), 2)+
//                               cos(rad_lat1)*
//                               cos(rad_lat2)*
//                               pow(sin(d_lon/2), 2)));
//   distance *= kEarthRadius;
//   return distance;  // 单位为Km
// }

bool IsPointInsidePolygon(LocationPoint const& check_point,
                          std::vector<LocationPoint> const& points) {
  int corss_cnt = 0;
  auto count = points.size();
  for (size_t i = 0; i < count; ++i) {
    auto const& point1 = points[i];
    auto const& point2 = points[(i+1)%count];
    if (check_point.latitude > LonLatMax(point1.latitude, point2.latitude) ||
        check_point.latitude < LonLatMin(point1.latitude, point2.latitude) ||
        check_point.longitude >
            LonLatMax(point1.longitude, point2.longitude) ||
        LonLatEqual(check_point.latitude,
            LonLatMax(point1.latitude, point2.latitude)) ||
        LonLatEqual(check_point.latitude,
            LonLatMin(point1.latitude, point2.latitude))) {
      continue;
    }
    double longitude = point1.longitude-
        (point1.longitude-point2.longitude)*
        (point1.latitude-check_point.latitude)/
        (point1.latitude-point2.latitude);
    if (longitude > check_point.longitude) {
      ++corss_cnt;
    }
  }
  return (corss_cnt%2 == 1);
}

}  // namespace

int main(int argc, char **argv) {
  // 检查参数.
  if (argc != 2) {
    print_usage(argv[0]);
    return -1;
  }
  // 从文件读取NMEA语句.
  // 注意: 文件中每组NMEA语句中必须包含GGA和RMC语句.
  std::vector<std::string> nmea_lines;
  std::ifstream ifs;
  std::string line;
  ifs.open(argv[1], std::ios::in);
  if (!ifs.is_open()) {
    printf("Open file failed!!!\n");
    return false;
  }
  while (getline(ifs, line)) {
    nmea_lines.push_back(line);
  }
  ifs.close();
  printf("Read %u line\n", nmea_lines.size());
  //
  // 808初始化.
  //
  // 协议参数
  libjt808::ProtocolParameter svr_para {};
  libjt808::ProtocolParameter cli_para {};
  cli_para.msg_head.phone_num = "13523339527";
  cli_para.msg_head.msg_flow_num = 1;
  svr_para.msg_head.phone_num = "13523339527";
  svr_para.msg_head.msg_flow_num = 1;
  // 命令生成器初始化.
  libjt808::Packager jt808_packager;
  libjt808::JT808FramePackagerInit(&jt808_packager);
  // 命令解析器初始化.
  libjt808::Parser jt808_parser;
  libjt808::JT808FrameParserInit(&jt808_parser);
  // 区域参数.
  libjt808::PolygonAreaSet polygon_area_set;
  // 先设定几个区域.
  std::vector<uint8_t> out;
  // 51:26.
  libjt808::LocationPoint location_point {114.099322,  22.543607, 0.0};
  SetPolygonArea(0x0001, location_point, &svr_para.polygon_area);
  svr_para.msg_head.msg_id = libjt808::kSetPolygonArea;
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++svr_para.msg_head.msg_flow_num;
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  polygon_area_set.insert(std::make_pair(cli_para.parse.polygon_area.area_id,
                                         cli_para.parse.polygon_area));
  // 51:31
  location_point.longitude = 114.098783;
  location_point.latitude = 22.543598;
  SetPolygonArea(0x0002, location_point, &svr_para.polygon_area);
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++svr_para.msg_head.msg_flow_num;
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  polygon_area_set.insert(std::make_pair(cli_para.parse.polygon_area.area_id,
                                         cli_para.parse.polygon_area));
  // 51:48.
  location_point.longitude = 114.096877;
  location_point.latitude = 22.543537;
  SetPolygonArea(0x0003, location_point, &svr_para.polygon_area);
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  ++svr_para.msg_head.msg_flow_num;
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  polygon_area_set.insert(std::make_pair(cli_para.parse.polygon_area.area_id,
                                         cli_para.parse.polygon_area));
  for (auto const& area : polygon_area_set) {
    printf("area id: %04X\n", area.second.area_id);
    printf("vertices: %04X\n", area.second.vertices.size());
    int i = 1;
    for (auto const& point: area.second.vertices) {
      printf("point%d: lon=%.6lf, lat=%.6lf\n",
             i, point.longitude, point.latitude);
      ++i;
    }
  }
  //
  // NMEA解析.
  //
  // 初始化.
  libnmeaparser::NmeaParser nmea_parser;
  nmea_parser.Init();
  nmea_parser.OnNmeaCallback([](const char *line) {
    printf("NMEA: %s\r\n", line);
  });
  uint32_t in_out_area_flag = 0;
  uint32_t last_in_out_area_id = 0;
  // 解析回调函数.
  nmea_parser.OnLocationCallback(
    [&](const libnmeaparser::GPSLocation &location) {
      printf("time: %s, status: %d, age: %f, accuracy: %f, "
             "log: %.6lf%c, lat: %.6lf%c, alt: %.2f, su: %d, "
             "speed: %f, bearing: %f\r\n",
             location.utc_time, location.position,
             location.age, location.accuracy,
             location.longitude, location.longitude_hemisphere,
             location.latitude, location.latitude_hemisphere,
             location.altitude, location.satellites_used,
             location.speed, location.bearing);
      LocationPoint point {location.longitude, location.latitude, 0.0};
      cli_para.location_info.alarm.bit.in_out_area = 0;
      for (auto const& area: polygon_area_set) {
        // 判断是否进入区域.
        if ((IsPointInsidePolygon(point, area.second.vertices)) &&
            area.second.area_attribute.bit.in_alarm_to_server) {
          if (in_out_area_flag%2 == 1) break;  // 未离开区域.
          cli_para.location_info.alarm.bit.in_out_area = 1;
          last_in_out_area_id = area.first;
          std::vector<uint8_t> item_value;
          if (libjt808::SetAccessAreaAlarmBody(
            libjt808::kAccessAreaAlarmPolygonArea, area.first,
            libjt808::kAccessAreaAlarmInArea, &item_value) == 0) {
            cli_para.location_extension.insert(
                std::make_pair(libjt808::kAccessAreaAlarm, item_value));
          }
          ++in_out_area_flag;
          break;
        }
      }
      // 判断是否离开区域.
      if ((cli_para.location_info.alarm.bit.in_out_area == 0) &&
          (in_out_area_flag%2 == 1)) {
        auto const& it = polygon_area_set.find(last_in_out_area_id);
        if ((it != polygon_area_set.end()) &&
            it->second.area_attribute.bit.out_alarm_to_server) {
          cli_para.location_info.alarm.bit.in_out_area = 1;
          std::vector<uint8_t> item_value;
          if (libjt808::SetAccessAreaAlarmBody(
            libjt808::kAccessAreaAlarmPolygonArea, last_in_out_area_id,
            libjt808::kAccessAreaAlarmOutArea, &item_value) == 0) {
            cli_para.location_extension.insert(
                std::make_pair(libjt808::kAccessAreaAlarm, item_value));
          }
        }
        ++in_out_area_flag;
        last_in_out_area_id = 0;
      }

      auto& status = cli_para.location_info.status;
      status.bit.gps_en = 1;
      status.bit.beidou_en = 1;
      status.bit.positioning = (location.position>0?1:0);
      status.bit.sn_latitude = (location.latitude_hemisphere=='N'?0:1);
      status.bit.ew_longitude = (location.longitude_hemisphere=='E'?0:1);
      cli_para.location_info.latitude =
          static_cast<uint32_t>(location.latitude*1e6);
      cli_para.location_info.longitude =
          static_cast<uint32_t>(location.longitude*1e6);
      cli_para.location_info.altitude =
          static_cast<uint16_t>(location.altitude);
      cli_para.location_info.speed = static_cast<uint16_t>(location.speed/10);
      cli_para.location_info.bearing = static_cast<uint16_t>(location.bearing);
      cli_para.location_info.time.clear();
      cli_para.location_info.time.assign(location.utc_time,
                                         location.utc_time+12);
      uint8_t temp = static_cast<uint8_t>(location.satellites_used);
      cli_para.location_extension.insert(std::make_pair(
          libjt808::kGnssSatellites, std::vector<uint8_t>{temp}));
      cli_para.location_extension.insert(std::make_pair(
          libjt808::kCustomInformationLength, std::vector<uint8_t>{1}));
      temp = static_cast<uint8_t>(location.position);
      cli_para.location_extension.insert(std::make_pair(
          libjt808::kPositioningStatus, std::vector<uint8_t>{temp}));
      cli_para.msg_head.msg_id = libjt808::kLocationReport;
      if (libjt808::JT808FramePackage(jt808_packager, cli_para, &out) < 0) {
        printf("Generate message failed\n");
        return;
      }
      for (auto& item : cli_para.location_extension) {
        item.second.clear();
      }
      cli_para.location_extension.clear();
      ++cli_para.msg_head.msg_flow_num;
      //
      // 解析消息.
      //
      for (auto& item : svr_para.parse.location_extension) {
        item.second.clear();
      }
      svr_para.parse.location_extension.clear();
      if (libjt808::JT808FrameParse(jt808_parser, out, &svr_para) == 0) {
        auto const& basic_info = svr_para.parse.location_info;
        auto const& extension_info = svr_para.parse.location_extension;
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
  });
  //
  // 开始解析NMEA语句.
  //
  for (auto const& line : nmea_lines) {
    nmea_parser.PutNmeaLine(line);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  //
  // 删除区域路线.
  //
  svr_para.polygon_area_id.push_back(0x0001);
  svr_para.polygon_area_id.push_back(0x0002);
  svr_para.polygon_area_id.push_back(0x0003);
  svr_para.msg_head.msg_id = libjt808::kDeletePolygonArea;
  if (libjt808::JT808FramePackage(jt808_packager, svr_para, &out) < 0) {
    printf("Generate message failed\n");
    return -1;
  }
  // printf("raw msg: ");
  // for (auto& uch : out) printf("0x%02X, ", uch);
  // printf("\n");
  ++svr_para.msg_head.msg_flow_num;
  cli_para.parse.polygon_area_id.clear();
  if (libjt808::JT808FrameParse(jt808_parser, out, &cli_para)) {
    printf("Parse message failed\n");
    return -1;
  }
  if ((cli_para.parse.msg_head.msg_id == libjt808::kDeletePolygonArea) &&
      !cli_para.parse.polygon_area_id.empty()) {
    for (auto const& id : cli_para.parse.polygon_area_id) {
      auto iter = polygon_area_set.find(id);
      if (iter != polygon_area_set.end()) {
        iter->second.vertices.clear();
        polygon_area_set.erase(iter);
      }
    }
    cli_para.parse.polygon_area_id.clear();
  }
  printf("area count: %d\n", polygon_area_set.size());
  for (auto const& area : polygon_area_set) {
    printf("area id: %04X\n", area.second.area_id);
    printf("vertices: %04X\n", area.second.vertices.size());
    int i = 1;
    for (auto const& point: area.second.vertices) {
      printf("point%d: lon=%.6lf, lat=%.6lf\n", i, point.longitude, point.latitude);
      ++i;
    }
  }
  nmea_lines.clear();
  return 0;
}
