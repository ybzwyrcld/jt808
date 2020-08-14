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

// @File    :  protocol_parameter.h
// @Version :  1.0
// @Time    :  2020/06/24 10:00:51
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_PROTOCOL_PARAMETER_H_
#define JT808_PROTOCOL_PARAMETER_H_

#include <string>
#include <vector>

#include "jt808/area_route.h"
#include "jt808/location_report.h"
#include "jt808/terminal_parameter.h"


namespace libjt808 {

// 已支持的协议命令.
enum SupportedCommands {
  kTerminalGeneralResponse = 0x0001,  // 终端通用应答.
  kPlatformGeneralResponse = 0x8001,  // 平台通用应答.
  kTerminalHeartBeat = 0x0002,  // 终端心跳.
  kFillPacketRequest = 0x8003,  // 补传分包请求.
  kTerminalRegister = 0x0100,  // 终端注册.
  kTerminalRegisterResponse = 0x8100,  // 终端注册应答.
  kTerminalLogOut = 0x0003,  // 终端注销.
  kTerminalAuthentication= 0x0102,  // 终端鉴权.
  kSetTerminalParameters = 0x8103,  // 设置终端参数.
  kGetTerminalParameters = 0x8104,  // 查询终端参数.
  kGetSpecificTerminalParameters = 0x8106,  // 查询指定终端参数.
  kGetTerminalParametersResponse = 0x0104,  // 查询终端参数应答.
  kTerminalUpgrade = 0x8108,  // 下发终端升级包.
  kTerminalUpgradeResultReport = 0x0108,  // 终端升级结果通知.
  kLocationReport = 0x0200,  // 位置信息汇报.
  kGetLocationInformation = 0x8201,  // 位置信息查询.
  kGetLocationInformationResponse = 0x0201,  // 位置信息查询应答.
  kLocationTrackingControl = 0x8202,  // 临时位置跟踪控制.
  kSetPolygonArea = 0x8604,  // 设置多边形区域.
  kDeletePolygonArea = 0x8605,  // 删除多边形区域.
};

// 所有应答命令.
constexpr uint16_t kResponseCommand[] = {
  kTerminalGeneralResponse,
  kPlatformGeneralResponse,
  kTerminalRegisterResponse,
  kGetTerminalParametersResponse,
  kGetLocationInformationResponse,
};

// 车牌颜色.
enum VehiclePlateColor {
  kVin = 0x0,  // 车辆未上牌.
  kBlue,
  kYellow,
  kBlack,
  kWhite,
  kOther
};

// 通用应答结果.
enum GeneralResponseResult {
  kSuccess = 0x0,  // 成功/确认.
  kFailure,  // 失败.
  kMessageHasWrong,  // 消息有误.
  kNotSupport,  // 不支持.
  kAlarmHandlingConfirmation,  // 报警处理确认, 仅平台应答使用.
};

// 注册应答结果.
enum RegisterResponseResult {
  kRegisterSuccess = 0x0,  // 成功.
  kVehiclesHaveBeenRegistered,  // 车辆已被注册.
  kNoSuchVehicleInTheDatabase,  // 数据库中无该车辆.
  kTerminalHaveBeenRegistered,  // 终端已被注册.
  kNoSuchTerminalInTheDatabase,  // 数据库中无该终端.
};

// 消息体属性.
union MsgBodyAttribute {
  struct {
    // 消息体长度, 占用10bit.
    uint16_t msglen:10;
    // 数据加密方式, 当此三位都为0, 表示消息体不加密,
    // 当第10位为1, 表示消息体经过RSA算法加密.
    uint16_t encrypt:3;
    // 分包标记.
    uint16_t packet:1;
    // 保留2位.
    uint16_t retain:2;
  }bit;
  uint16_t u16val;
};

// 消息内容起始位置.
enum MsgBodyPos {
  MSGBODY_NOPACKET_POS = 13,  // 短消息体消息内容起始位置.
  MSGBODY_PACKET_POS = 17,  // 长消息体消息内容起始位置.
};

// 转义相关标识.
enum ProtocolEscapeFlag {
  PROTOCOL_SIGN = 0x7E,  // 标识位.
  PROTOCOL_ESCAPE = 0x7D,  // 转义标识.
  PROTOCOL_ESCAPE_SIGN = 0x02,  // 0x7E<-->0x7D后紧跟一个0x02.
  PROTOCOL_ESCAPE_ESCAPE = 0x01,  // 0x7D<-->0x7D后紧跟一个0x01.
};

// 消息头.
struct MsgHead {
  // 消息ID.
  uint16_t msg_id;
  // 消息体属性.
  MsgBodyAttribute msgbody_attr;
  // 终端手机号.
  std::string phone_num;
  // 消息流水号.
  uint16_t msg_flow_num;
  // 总包数, 分包情况下使用.
  uint16_t total_packet;
  // 当前包编号, 分包情况下使用.
  uint16_t packet_seq;
};

// 注册信息.
struct RegisterInfo {
  // 省域ID.
  uint16_t province_id;
  // 市县域ID.
  uint16_t city_id;
  // 制造商ID, 固定5个字节.
  std::vector<uint8_t> manufacturer_id;
  // 终端型号, 固定20个字节, 位数不足后补0x00.
  std::vector<uint8_t> terminal_model;
  // 终端ID, 固定7个字节, 位数不足后补0x00.
  std::vector<uint8_t> terminal_id;
  // 车牌颜色, 0表示未上牌.
  uint8_t car_plate_color;
  // 车辆标识, 仅在上牌时使用.
  std::string car_plate_num;
  // 重载拷贝赋值操作符.
  void operator=(const RegisterInfo& info) {
    province_id = info.province_id;
    city_id = info.city_id;
    manufacturer_id.clear();
    manufacturer_id.assign(info.manufacturer_id.begin(),
                           info.manufacturer_id.end());
    terminal_model.clear();
    terminal_model.assign(info.terminal_model.begin(),
                           info.terminal_model.end());
    terminal_id.clear();
    terminal_id.assign(info.terminal_id.begin(), info.terminal_id.end());
    car_plate_num.clear();
    car_plate_color = info.car_plate_color;
    if (car_plate_color != kVin) {
      car_plate_num.assign(info.car_plate_num.begin(),
                           info.car_plate_num.end());
    }
  }
};

// 升级类型.
enum kTerminalUpgradeType {
  // 终端.
  kTerminal = 0x0,
  // 道路运输证 IC 卡读卡器
  kICCardReader = 0xc,
  // 北斗卫星定位模块.
  kGNSS = 0x34,
};

// 升级结果.
enum kTerminalUpgradeResultType {
  // 终端升级成功.
  kTerminalUpgradeSuccess = 0x0,
  // 终端升级失败.
  kTerminalUpgradeFailed,
  // 终端升级取消.
  kTerminalUpgradeCancel
};

// 升级信息.
struct UpgradeInfo {
  // 升级类型.
  uint8_t upgrade_type;
  // 升级结果.
  uint8_t upgrade_result;
  // 制造商ID, 固定5个字节.
  std::vector<uint8_t> manufacturer_id;
  // 升级版本号.
  std::string version_id;
  // 升级数据包.
  std::vector<uint8_t> upgrade_data;
};

// 补传分包信息.
struct FillPacket {
  // 分包数据首包的消息流水号.
  uint16_t first_packet_msg_flow_num;
  // 需要重传包的ID.
  std::vector<uint16_t> packet_id;
};

// 协议所有参数.
struct ProtocolParameter {
  uint8_t respone_result;
  uint16_t respone_msg_id;
  uint16_t respone_flow_num;
  // 消息头.
  MsgHead msg_head;
  // 终端注册时需填充注册信息.
  RegisterInfo register_info;
  // 平台随机生成鉴权码.
  std::vector<uint8_t> authentication_code;
  // 设置终端参数项.
  TerminalParameters terminal_parameters;
  // 查询终端参数ID列表.
  std::vector<uint32_t> terminal_parameter_ids;
  // 位置上报时填充位置基本信息, 必须项.
  LocationBasicInformation location_info;
  // 位置上报时填充位置附加信息, 可选项.
  LocationExtensions location_extension;
  // 临时位置跟踪控制信息.
  LocationTrackingControl location_tracking_control;
  // 多边形区域集.
  // PolygonAreaSet polygon_area_set;
  // 多边形区域.
  PolygonArea polygon_area;
  // 删除多边形区域ID集.
  std::vector<uint32_t> polygon_area_id;
  // 升级信息.
  UpgradeInfo upgrade_info;
  // 补传分包信息.
  struct FillPacket fill_packet;
  // 保留字段.
  std::vector<uint8_t> retain;
  // 用于解析消息.
  struct {
    uint8_t respone_result;
    uint16_t respone_msg_id;
    uint16_t respone_flow_num;
    // 解析出的消息头.
    MsgHead msg_head;
    // 解析出的注册信息.
    RegisterInfo register_info;
    // 解析出的鉴权码.
    std::vector<uint8_t> authentication_code;
    // 解析出的设置终端参数项.
    TerminalParameters terminal_parameters;
    // 解析出的查询终端参数ID列表.
    std::vector<uint32_t> terminal_parameter_ids;
    // 解析出的位置基本信息.
    LocationBasicInformation location_info;
    // 解析出的位置附加信息.
    LocationExtensions location_extension;
    // 解析出的临时位置跟踪控制信息.
    LocationTrackingControl location_tracking_control;
    // 解析出的多边形区域集.
    // PolygonAreaSet polygon_area_set;
    // 解析出的多边形区域.
    PolygonArea polygon_area;
    // 解析出的删除多边形区域ID集.
    std::vector<uint32_t> polygon_area_id;
    // 解析出的升级信息.
    UpgradeInfo upgrade_info;
    // 解析出的补传分包信息.
    struct FillPacket fill_packet;
    // 解析出的保留字段.
    std::vector<uint8_t> retain;
  }parse;
};

}  // namespace libjt808

#endif  // JT808_PROTOCOL_PARAMETER_H_
