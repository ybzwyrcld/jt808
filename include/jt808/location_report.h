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

// @File    :  location_report.h
// @Version :  1.0
// @Time    :  2020/07/02 09:56:33
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_LOCATION_REPORT_H_
#define JT808_LOCATION_REPORT_H_

#include <stdint.h>

#include <string>
#include <vector>


namespace libjt808 {

// 报警标志位
union AlarmBit {
  struct {
    // 紧急报瞥触动报警开关后触发.
    uint32_t sos:1;
    // 超速报警.
    uint32_t overspeed:1;
    // 疲劳驾驶.
    uint32_t fatigue:1;
    // 预警.
    uint32_t early_warning:1;
    // GNSS模块发生故障.
    uint32_t gnss_fault:1;
    // GNSS天线未接或被剪断.
    uint32_t gnss_antenna_cut:1;
    // GNSS天线短路.
    uint32_t gnss_antenna_shortcircuit:1;
    // 终端主电源欠压.
    uint32_t power_low:1;
    // 终端主电源掉电.
    uint32_t power_cut:1;
    // 终端LCD或显示器故障.
    uint32_t lcd_fault:1;
    // TTS模块故障.
    uint32_t tts_fault:1;
    // 摄像头故障.
    uint32_t camera_fault:1;
    // OBD故障码.
    uint32_t obd_fault_code:1;
    // 保留5位.
    uint32_t retain1:5;
    // 当天累计驾驶超时.
    uint32_t day_drive_overtime:1;
    // 超时停车.
    uint32_t stop_driving_overtime:1;
    // 进出区域.
    uint32_t in_out_area:1;
    // 进出路线.
    uint32_t in_out_road:1;
    // 路段行驶时间不足/过长.
    uint32_t road_drive_time:1;
    // 路线偏离报警.
    uint32_t road_deviate:1;
    // 车辆VSS故障.
    uint32_t vss_fault:1;
    // 车辆油量异常.
    uint32_t oil_fault:1;
    // 车辆被盗(通过车辆防盗器).
    uint32_t car_alarm:1;
    // 车辆非法点火.
    uint32_t car_acc_alarm:1;
    // 车辆非法位移.
    uint32_t car_move:1;
    // 碰撞侧翻报警.
    uint32_t collision:1;
    // 保留2位.
    uint32_t retain2:2;
  }bit;
  uint32_t value;
};

// 状态位
union StatusBit {
  struct {
    // ACC开关, 0:ACC关; 1:ACC开.
    uint32_t acc:1;
    // 定位标志, 0:未定位; 1:定位.
    uint32_t positioning:1;
    // 纬度半球, 0:北纬: 1:南纬.
    uint32_t sn_latitude:1;
    // 经度半球, 0:东经; 1:西经.
    uint32_t ew_longitude:1;
    // 0:运营状态; 1:停运状态.
    uint32_t operation:1;  // 0:运营状态; 1:停运状态.
    // 0:经纬度未经保密插件加密; 1:经纬度已经保密插件加密.
    uint32_t gps_encrypt:1;
    // 保留2位.
    uint32_t retain1:2;
    // 00: 空车; 01: 半载; 10: 保留; 11: 满载.
    uint32_t trip_status:2;
    // 0:车辆油路正常; 1:车辆油路断开.
    uint32_t oil_cut:1;
    // 0:车辆电路正常; 1:车辆电路断开.
    uint32_t circuit_cut:1;
    // 0:车门解锁; 1: 车门加锁.
    uint32_t door_lock:1;
    // 0:门1 关; 1: 门1 开; (前门).
    uint32_t door1_status:1;
    // 0:门2 关; 1: 门2 开; (中门).
    uint32_t door2_status:1;
    // 0:门 3 关; 1: 门 3 开; (后门).
    uint32_t door3_status:1;
    // 0:门 4 关; 1: 门 4 开; (驾驶席门).
    uint32_t door4_status:1;
    // 0:门 5 关; 1: 门 5 开; (自定义).
    uint32_t door5_status:1;
    // 0: 未使用 GPS 卫星进行定位; 1: 使用 GPS 卫星进行定位.
    uint32_t gps_en:1;
    // 0: 未使用北斗卫星进行定位; 1: 使用北斗卫星进行定位.
    uint32_t beidou_en:1;
    // 0: 未使用 GLONASS 卫星进行定位; 1: 使用 GLONASS 卫星进行定位.
    uint32_t glonass_en:1;
    // 0: 未使用 Galileo 卫星进行定位; 1: 使用 Galileo 卫星进行定位.
    uint32_t galileo_en:1;
    // 保留10位.
    uint32_t retain2:10;
  }bit;
  uint32_t value;
};

// 位置基本信息数据.
struct LocationBasicInformation {
  union AlarmBit alarm;
  union StatusBit status;
  // 纬度(以度为单位的纬度值乘以10的6次方, 精确到百万分之一度)
  uint32_t latitude;
  // 经度(以度为单位的纬度值乘以10的6次方, 精确到百万分之一度)
  uint32_t longitude;
  // 海拔高度, 单位为米(m)
  uint16_t altitude;
  // 速度 1/10km/h
  uint16_t speed;
  // 方向 0-359,正北为0, 顺时针
  uint16_t bearing;
  // 时间, "YYMMDDhhmmss"(GMT+8时间, 本标准之后涉及的时间均采用此时区).
  std::string time;
};

// 扩展车辆信号状态位
union ExtendedVehicleSignalBit {
  struct {
    // 近光灯信号
    uint32_t near_lamp:1;
    // 远光灯信号
    uint32_t farl_amp:1;
    // 右转向灯信号
    uint32_t right_turn_lamp:1;
    // 左转向灯信号
    uint32_t left_turn_lamp:1;
    // 制动信号
    uint32_t breaking:1;
    // 倒档信号
    uint32_t reversing:1;
    // 雾灯信号
    uint32_t fog_lamp:1;
    // 示廓灯
    uint32_t outline_lamp:1;
    // 喇叭信号
    uint32_t horn:1;
    // 空调状态
    uint32_t air_conditioner:1;
    // 空挡信号
    uint32_t neutral:1;
    // 缓速器工作
    uint32_t retarder:1;
    // ABS 工作
    uint32_t abs:1;
    // 加热器工作
    uint32_t heater:1;
    // 离合器状态
    uint32_t clutch:1;
    // 保留17位.
    uint32_t retain:17;
  }bit;
  uint32_t value;
};

// 位置信息上报附加项ID.
enum LocationExtensionId {
  // 里程, 1/10km, 对应车上里程表读数, DWORD
  kMileage = 0x01,
  // 油量, 1/10L, 对应车上油量表读数, WORD
  kOilMass = 0x02,
  // 行驶记录功能获取的速度, 1/10km/h, WORD
  kTachographSpeed = 0x03,
  // 需要人工确认报警事件的 ID, 从 1 开始计数, WORD
  kAlarmCount = 0x04,
  // 超速报警附加信息, BYTE or BYTE+DWORD
  kOverSpeedAlarm = 0x11,
  // 进出区域/路线报警附加信息, BYTE+DWORD+BYTE
  kAccessAreaAlarm = 0x12,
  // 路段行驶时间不足/过长报警附加信息, DWORD+WORD+BYTE
  kDrivingTimeAlarm = 0x13,
  // 扩展车辆信号状态位, DWORD
  kVehicleSignalStatus = 0x25,
  // IO 状态位, WORD
  kIoStatus = 0x2A,
  // 模拟量, DWORD
  kAnalogQuantity = 0x2B,
  // 无线通信网络信号强度, BYTE
  kNetworkQuantity = 0x30,
  // GNSS 定位卫星数, BYTE
  kGnssSatellites = 0x31,
  // 后续自定义信息长度, BYTE
  kCustomInformationLength = 0xE0,
  // 定位解状态, BYTE
  kPositioningStatus = 0xEE
};

//  超速报警附加信息位置类型, BYTE.
enum kOverSpeedAlarmLocationType {
  // 无特定位置.
  kOverSpeedAlarmNoSpecificLocation=0x0,
  // 圆形区域.
  kOverSpeedAlarmCircularArea,
  // 矩形区域.
  kOverSpeedAlarmRectangleArea,
  // 多边形区域.
  kOverSpeedAlarmPolygonArea,
  // 路段.
  kOverSpeedAlarmRoadSection
};

// 进出区域/路线报警附加信息消息体位置类型, BYTE.
enum kAccessAreaAlarmLocationType {
  // 圆形区域.
  kAccessAreaAlarmCircularArea=0x0,
  // 矩形区域.
  kAccessAreaAlarmRectangleArea,
  // 多边形区域.
  kAccessAreaAlarmPolygonArea,
  // 路线.
  kOverSpeedAlarmRoute
};

// 进出区域/路线报警附加信息消息体方向类型, BYTE.
enum kAccessAreaAlarmDirectionType {
  // 进入区域.
  kAccessAreaAlarmInArea=0x0,
  // 离开区域.
   kAccessAreaAlarmOutArea
};

// IO 状态位
union IoStatusBit {
  struct {
    // 深度休眠状态
    uint16_t deep_rmancy:1;
    // 休眠状态
    uint16_t dormancy:1;
    // 保留14位.
    uint16_t retain:14;
  }bit;
  uint16_t value;
};

// 临时位置跟踪控制信息.
struct LocationTrackingControl {
  // 时间间隔.
  uint16_t interval;
  // 单位为秒(s), 有效时间.
  uint32_t tracking_time;
};

// 设置超速报警附加信息消息体.
int SetOverSpeedAlarmBody(uint8_t const& location_type,
                          uint32_t const& area_route_id,
                          std::vector<uint8_t>* out);

// 获得超速报警报警附加信息消息体.
int GetOverSpeedAlarmBody(std::vector<uint8_t> const& out,
                          uint8_t* location_type,
                          uint32_t* area_route_id);

// 设置进出区域/路线报警附加信息消息体.
int SetAccessAreaAlarmBody(uint8_t const& location_type,
                           uint32_t const& area_route_id,
                           uint8_t const& direction,
                           std::vector<uint8_t>* out);

// 获得进出区域/路线报警附加信息消息体.
int GetAccessAreaAlarmBody(std::vector<uint8_t> const& out,
                           uint8_t* location_type,
                           uint32_t* area_route_id,
                           uint8_t* direction);

}  // namespace libjt808

#endif  // JT808_LOCATION_REPORT_H_
