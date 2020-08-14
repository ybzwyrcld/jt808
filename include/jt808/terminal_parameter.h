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

// @File    :  terminal_parameter.h
// @Version :  1.0
// @Time    :  2020/07/15 16:01:15
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_TERMINAL_PARAMETER_H_
#define JT808_TERMINAL_PARAMETER_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "jt808/util.h"


namespace libjt808 {

// 终端参数设置项参数ID.
enum TerminalParameterID {
  // DWORD, 终端心跳发送间隔(s).
  kTerminalHeartBeatInterval = 0x0001,
  // DWORD, TCP消息应答超时时间(s).
  kTCPResponseTimeout = 0x0002,
  // DWORD, TCP消息重传次数.
  kTCPMsgRetransmissionTimes = 0x0003,
  // DWORD, UDP消息应答超时时间(s).
  kUDPResponseTimeout = 0x0004,
  // DWORD, UDP消息重传次数
  kUDPMsgRetransmissionTimes = 0x0005,
  // DWORD, SMS消息应答超时时间(s).
  kSMSResponseTimeout = 0x0006,
  // DWORD, SMS消息重传次数.
  kSMSMsgRetransmissionTimes = 0x0007,
  // DWORD, 位置汇报策略, 0:定时汇报; 1:定距汇报; 2:定时和定距汇报.
  kLocationReportWay = 0x0020,
  // DWORD, 位置汇报方案, 0:根据 ACC 状态; 1:根据登录状态和ACC
  //        状态, 先判断登录状态, 若登录再根据 ACC 状态.
  kLocationReportPlan = 0x0021,
  // DWORD, 驾驶员未登录汇报时间间隔.
  kDriverNotLoginReportTimeInterval = 0x0022,
  // DWORD, 休眠时汇报时间间隔
  kSleepingReportTimeInterval = 0x0027,
  // DWORD, 紧急报警时汇报时间间隔
  kAlarmingReportTimeInterval = 0x0028,
  // DWORD, 缺省时间汇报间隔
  kDefaultTimeReportTimeInterval = 0x0029,
  // DWORD, 缺省距离汇报间隔
  kDefaultReportDistanceInterval = 0x002C,
  // DWORD, 驾驶员未登录汇报距离间隔.
  kDriverNotLoginReportDistanceInterval = 0x002D,
  // DWORD, 休眠时汇报距离间隔
  kSleepingReportDistanceInterval = 0x002E,
  // DWORD, 紧急报警时汇报距离间隔
  kAlarmingReportDistanceInterval = 0x002F,
  // DWORD, 拐点补传角度, < 180°.
  kCornerPointRetransmissionAngle = 0x0030,
  // DWORD, 报警屏蔽字, 与位置信息汇报消息中的报警标志相对
  //        应, 相应位为1则相应报警被屏蔽.
  kAlarmShieldWord = 0x0050,
  // DWORD, 报警发送文本SMS开关, 与位置信息汇报消息中的报警
  //        标志相对应, 相应位为1则相应报警时发送文本SMS.
  kAlarmSendSMSText = 0x0051,
  // DWORD, 报警拍摄开关, 与位置信息汇报消息中的报警标志相
  //        对应, 相应位为1则相应报警时摄像头拍摄.
  kAlarmShootSwitch = 0x0052,
  // DWORD, 报警拍摄存储标志, 与位置信息汇报消息中的报警标
  //        志相对应, 相应位为1则对相应报警时拍的照片进行
  //        存储, 否则实时上传.
  kAlarmShootSaveFlag= 0x0053,
  // DWORD, 关键标志, 与位置信息汇报消息中的报警标志相对应,
  //        相应位为1则对相应报警为关键报警.
   kAlarmKeyFlag= 0x0054,
  // DWORD, 最高速度, km/h.
  kMaxSpeed = 0x0055,
  // BYTE,  GNSS定位模式, 定义如下:
  //        bit0, 0: 禁用GPS定位, 1: 启用GPS定位;
  //        bit1, 0: 禁用北斗定位, 1: 启用北斗定位;
  //        bit2, 0: 禁用GLONASS定位, 1: 启用GLONASS定位;
  //        bit3, 0: 禁用Galileo定位, 1: 启用Galileo定位.
  kGNSSPositionMode = 0x0090,
  // BYTE,  GNSS波特率, 定义如下:
  //        0x00: 4800; 0x01: 9600: 0x02: 19200;
  //        0x03: 38400; 0x04: 57600; 0x05: 115200.
  kGNSSBaudeRate = 0x0091,
  // BYTE,  GNSS模块详细定位数据输出频率, 定义如下:
  //        0x00: 500ms; 0x01: 1000ms(默认值);
  //        0x02: 2000ms; 0x03: 3000ms; 0x04: 4000m.
  kGNSSOutputFrequency = 0x0092,
  // DWORD, GNSS模块详细定位数据采集频率(s), 默认为1.
  kGNSSOutputCollectFrequency = 0x0093,
  // BYTE,  GNSS模块详细定位数据上传方式:
  //        0x00, 本地存储, 不上传(默认值);
  //        0x01, 按时间间隔上传;
  //        0x02, 按距离间隔上传;
  //        0x0B, 按累计时间上传, 达到传输时间后自动停止上传;
  //        0x0C, 按累计距离上传, 达到距离后自动停止上传;
  //        0x0D, 按累计条数上传, 达到上传条数后自动停止上传.
  kGNSSOutputUploadWay = 0x0094,
  // DWORD, GNSS模块详细定位数据上传设置:
  //        上传方式为0x01时, 单位为秒;
  //        上传方式为0x02时, 单位为米;
  //        上传方式为0x0B时, 单位为秒;
  //        上传方式为0x0C时, 单位为米;
  //        上传方式为0x0D时, 单位为条.
  kSetGNSSDataUpload = 0x0095,
  // DWORD,   CAN总线通道1采集时间间隔(ms), 0表示不采集.
  kCANBus1CollectInterval = 0x0100,
  // WORD,    CAN总线通道1上传时间间隔(s), 0表示不上传.
  kCANBus1UploadInterval = 0x0101,
  // DWORD,   CAN总线通道2采集时间间隔(ms), 0表示不采集.
  kCANBus2CollectInterval = 0x0102,
  // WORD,    CAN总线通道2上传时间间隔(s), 0表示不上传.
  kCANBus2UploadInterval = 0x0103,
  // BYTE[8], CAN总线ID单独采集设置:
  //          bit63-bit32 表示此ID采集时间间隔(ms), 0表示不采集;
  //          bit31 表示CAN通道号, 0: CAN1, 1: CAN2;
  //          bit30 表示帧类型, 0: 标准帧, 1: 扩展帧;
  //          bit29 表示数据采集方式, 0: 原始数据, 1: 采集区间的计算值;
  //          bit28-bit0 表示CAN总线ID.
  kSetCANBusSpecial = 0x0110,
};

// GNSS模块数据输出波特率.
enum GNSSModelBaudeRate {
  kBDRT4800 = 0x0,
  kBDRT9600,
  kBDRT19200,
  kBDRT38400,
  kBDRT57600,
  kBDRT115200,
};

// 终端参数存储定义: key: itemid, value: itemvalue.
using TerminalParameters = std::map<uint32_t, std::vector<uint8_t>>;

//
//  终端参数转换.
//
// undefined type --> terminal parameter item.
template<typename T>
inline int SetTerminalParameter(
    uint32_t const& key, T const& value, TerminalParameters* items) {
  return -1;
}

// terminal parameter item --> undefined type.
template<typename T>
inline int GetTerminalParameter(
    TerminalParameters const& items, uint32_t const& key, T* value) {
  return -1;
}

// uint8_t --> terminal parameter item.
template<>
inline int SetTerminalParameter(
    uint32_t const& key, uint8_t const& value, TerminalParameters* items) {
  if (items == nullptr) return -1;
  auto it = items->find(key);
  if (it != items->end()) {
    it->second.clear();
    it->second.push_back(value);
  } else {
    items->insert(std::make_pair(key, std::vector<uint8_t>{value}));
  }
  return 0;
}

// terminal parameter item --> uint8_t.
template<>
inline int GetTerminalParameter(
    TerminalParameters const& items, uint32_t const& key, uint8_t* value) {
  if (value == nullptr) return -1;
  auto const& it = items.find(key);
  if (it != items.end()) {
    if (it->second.size() != 1) return -1;
    *value = it->second[0];
    return 0;
  }
  return -1;
}

// uint16_t --> terminal parameter item.
template<>
inline int SetTerminalParameter(
    uint32_t const& key, uint16_t const& value, TerminalParameters* items) {
  if (items == nullptr) return -1;
  U16ToU8Array cvt;
  cvt.u16val = EndianSwap16(value);
  auto it = items->find(key);
  if (it != items->end()) {
    it->second.clear();
    it->second.assign(cvt.u8array, cvt.u8array+2);
  } else {
    items->insert(std::make_pair(key,
        std::vector<uint8_t>(cvt.u8array, cvt.u8array+2)));
  }
  return 0;
}

// terminal parameter item --> uint16_t.
template<>
inline int GetTerminalParameter(
    TerminalParameters const& items, uint32_t const& key, uint16_t* value) {
  if (value == nullptr) return -1;
  auto const& it = items.find(key);
  if (it != items.end()) {
    if (it->second.size() != 2) return -1;
    *value = it->second[0]*256+it->second[1];
    return 0;
  }
  return -1;
}

// uint32_t --> terminal parameter item.
template<>
inline int SetTerminalParameter(
    uint32_t const& key, uint32_t const& value, TerminalParameters* items) {
  if (items == nullptr) return -1;
  U32ToU8Array cvt;
  cvt.u32val = EndianSwap32(value);
  auto it = items->find(key);
  if (it != items->end()) {
    it->second.clear();
    it->second.assign(cvt.u8array, cvt.u8array+4);
  } else {
    items->insert(std::make_pair(key,
        std::vector<uint8_t>(cvt.u8array, cvt.u8array+4)));
  }
  return 0;
}

// terminal parameter item --> uint32_t.
template<>
inline int GetTerminalParameter(
    TerminalParameters const& items, uint32_t const& key, uint32_t* value) {
  if (value == nullptr) return -1;
  auto const& it = items.find(key);
  if (it != items.end()) {
    if (it->second.size() != 4) return -1;
    *value = it->second[0]*65536*256+
             it->second[1]*65536+
             it->second[2]*256+
             it->second[3];
    return 0;
  }
  return -1;
}

// std::string --> terminal parameter item.
template<>
inline int SetTerminalParameter(
    uint32_t const& key, std::string const& value, TerminalParameters* items) {
  if (items == nullptr) return -1;
  auto it = items->find(key);
  if (it != items->end()) {
    it->second.clear();
    it->second.assign(value.begin(), value.end());
  } else {
    items->insert(std::make_pair(key,
        std::vector<uint8_t>(value.begin(), value.end())));
  }
  return 0;
}

// terminal parameter item --> std::string.
template<>
inline int GetTerminalParameter(
    TerminalParameters const& items, uint32_t const& key, std::string* value) {
  if (value == nullptr) return -1;
  auto const& it = items.find(key);
  if (it != items.end()) {
    value->clear();
    value->assign(it->second.begin(), it->second.end());
    return 0;
  }
  return -1;
}

//
//  协议支持的终端参数项封装/解析.
//

// 封装终端心跳发送间隔配置.
// Args:
//    intv:  终端心跳时间间隔.
// Returns:
//    成功返回0, 失败返回-1.
inline int PackagingTerminalParameterTerminalHeartBeatInterval(
    uint32_t const& intv, TerminalParameters* items) {
  return SetTerminalParameter(kTerminalHeartBeatInterval, intv, items);
}

// 解析终端心跳发送间隔配置.
// Args:
//    items:  终端参数项的map容器.
//    intv:  保存解析的终端心跳时间间隔.
// Returns:
//    成功返回0, 失败返回-1.
inline int ParseTerminalParameterTerminalHeartBeatInterval(
    TerminalParameters const& items, uint32_t* intv) {
  return GetTerminalParameter(items, kTerminalHeartBeatInterval, intv);
}

}  // namespace libjt808

#endif  // JT808_TERMINAL_PARAMETER_H_
