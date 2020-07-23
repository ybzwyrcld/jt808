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

// @File    :  area_route.h
// @Version :  1.0
// @Time    :  2020/07/06 15:23:00
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_AREA_ROUTE_H_
#define JT808_AREA_ROUTE_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>


namespace libjt808 {

// 区域属性.
union AreaAttribute {
  struct {
    // 1: 根据时间.
    uint16_t by_time:1;
    // 1: 限速.
    uint16_t speed_limit:1;
    // 1: 进区域报警给驾驶员.
    uint16_t in_alarm_to_dirver:1;
    // 1: 进区域报警给平台.
    uint16_t in_alarm_to_server:1;
    // 1: 出区域报警给驾驶员.
    uint16_t out_alarm_to_dirver:1;
    // 1: 出区域报警给平台.
    uint16_t out_alarm_to_server:1;
    // 0: 北纬:1; 1: 南纬.
    uint16_t sn_latitude:1;
    // 0: 东经:1; 1: 西经.
    uint16_t ew_longitude:1;
    // 0: 允许开门:1; 1: 禁止开门.
    uint16_t can_open_door:1;
    // 保留5位.
    uint16_t retain:5;
    // 0: 进区域开启通信模块; 1: 进区域关闭通信模块.
    uint16_t in_open_communication:1;
    // 0: 进区域不采集 GNSS 详细定位数据; 1: 进区域采集 GNSS 详细定位数据.
    uint16_t in_not_collect_gnss:1;
  }bit;
  uint16_t value;
};

// 地图坐标点.
struct LocationPoint {
  double longitude;
  double latitude;
  float altitude;
};

// 多边形区域信息.
struct PolygonArea {
  // 区域ID.
  uint32_t area_id;
  // 区域属性.
  AreaAttribute area_attribute;
  // 格式为"YYMMDDhhmmss", 若区域属性 0 位为 0 则没有该字段.
  std::string start_time;
  // 格式为"YYMMDDhhmmss", 若区域属性 0 位为 0 则没有该字段.
  std::string stop_time;
  // 单位为公里每小时(km/h), 若区域属性 1 位为 0 则没有该字段.
  uint16_t max_speed;
 // 超速持续时间, 单位为秒(s), 若区域属性 1 位为 0 则没有该字段.
  uint8_t overspeed_time;
  // 多边形区域的顶点项.
  std::vector<LocationPoint> vertices;
};

// 多边形区域集, map<区域ID, 区域信息>.
using PolygonAreaSet = std::map<uint32_t, PolygonArea>;

}  // namespace libjt808

#endif  // JT808_AREA_ROUTE_H_
