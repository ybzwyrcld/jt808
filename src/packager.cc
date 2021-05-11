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

// @File    :  packager.cc
// @Version :  1.0
// @Time    :  2020/07/08 16:52:05
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "packager.h"

#include "bcd.h"
#include "util.h"


namespace libjt808 {

namespace {

// 封装消息头.
int JT808FrameHeadPackage(MsgHead const& msg_head,
                          std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  out->clear();
  out->push_back(PROTOCOL_SIGN);  // 协议头部标识.
  U16ToU8Array u16converter;
  // 消息ID.
  u16converter.u16val = EndianSwap16(msg_head.msg_id);
  for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
  // 消息体属性.
  u16converter.u16val =  EndianSwap16(msg_head.msgbody_attr.u16val);
  for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
  // 终端手机号(BCD码).
  std::vector<uint8_t> phone_num_bcd;
  if (StringToBcd(msg_head.phone_num, &phone_num_bcd) != 0) return -1;
  for (auto& u8val : phone_num_bcd) out->push_back(u8val);
  // 消息流水号.
  u16converter.u16val = EndianSwap16(msg_head.msg_flow_num);
  for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
  // 封包项.
  if ((msg_head.msgbody_attr.bit.packet == 1) &&
      (msg_head.total_packet > 1)) {
    u16converter.u16val = EndianSwap16(msg_head.total_packet);
    for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
    u16converter.u16val = EndianSwap16(msg_head.packet_seq);
    for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
  }
  return 0;
}

// 消息内容长度修正.
int JT808MsgBodyLengthFix(MsgHead const& msg_head,
                          uint16_t const& msg_len,
                          std::vector<uint8_t>* out) {
  if (out == nullptr || out->size() < 12) return -1;
  auto msgbody_attr = msg_head.msgbody_attr;
  msgbody_attr.bit.msglen = msg_len;
  U16ToU8Array u16converter;
  u16converter.u16val = EndianSwap16(msgbody_attr.u16val);
  (*out)[3] = u16converter.u8array[0];
  (*out)[4] = u16converter.u8array[1];
  return 0;
}

// JT808协议转义.
int JT808MsgEscape(std::vector<uint8_t>* out) {
  *(out->begin()) = 0x00;
  *(out->end()-1) = 0x00;
  auto in = std::move(*out);
  if (Escape(in, out) < 0) return -1;
  *(out->begin()) = PROTOCOL_SIGN;
  *(out->end()-1) = PROTOCOL_SIGN;
  return 0;
}

}  // namespace

// 命令封装器初始化.
int JT808FramePackagerInit(Packager* packager) {
  // 0x0001, 终端通用应答.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kTerminalGeneralResponse,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 5;
        U16ToU8Array u16converter;
        // 应答消息流水号.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_flow_num);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 应答消息ID.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_id);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 应答结果.
        out->push_back(para.respone_result);
        return msg_len;
      }
  ));
  // 0x8001, 平台通用应答.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kPlatformGeneralResponse,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 5;
        U16ToU8Array u16converter;
        // 应答消息流水号.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_flow_num);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 应答消息ID.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_id);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 应答结果.
        out->push_back(para.respone_result);
        return msg_len;
      }
  ));
  // 0x0002, 终端心跳.
  packager->insert(std::pair<uint16_t, PackageHandler>(kTerminalHeartBeat,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x8003, 补传分包请求.
  packager->insert(std::pair<uint16_t, PackageHandler>(kFillPacketRequest,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 0;
        auto const& fill_packet = para.fill_packet;
        // 首包流水号.
        U16ToU8Array u16convert;
        u16convert.u16val = EndianSwap16(
            fill_packet.first_packet_msg_flow_num);
        for (int i = 0; i < 2; ++i) out->push_back(u16convert.u8array[i]);
        msg_len += 2;
        // 重传包总数.
        uint8_t cnt = fill_packet.packet_id.end() - fill_packet.packet_id.begin();
        out->push_back(cnt);
        ++msg_len;
        // 重传包ID.
        for (auto const& id : fill_packet.packet_id) {
          u16convert.u16val = EndianSwap16(id);
          for (int i = 0; i < 2; ++i) out->push_back(u16convert.u8array[i]);
          msg_len += 2;
        }
        return msg_len;
      }
  ));
  // 0x0100, 终端注册.
  packager->insert(std::pair<uint16_t, PackageHandler>(kTerminalRegister,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 37;
        auto& register_info = para.register_info;
        U16ToU8Array u16converter;
        // 省域ID.
        u16converter.u16val = EndianSwap16(register_info.province_id);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 市县域ID.
        u16converter.u16val = EndianSwap16(register_info.city_id);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 制造商ID.
        for (auto& ch: register_info.manufacturer_id) out->push_back(ch);
        // 终端型号.
        for (auto& ch: register_info.terminal_model) out->push_back(ch);
        if (register_info.terminal_model.size() < 20) {  // 长度不足补0x00.
          size_t size = 20-register_info.terminal_model.size();
          for (size_t i = 0; i < size; ++i) out->push_back(0x00);
        }
        // 终端ID.
        for (auto& ch: register_info.terminal_id) out->push_back(ch);
        if (register_info.terminal_id.size() < 7) {  // 长度不足补0x00.
          size_t size = 7-register_info.terminal_id.size();
          for (size_t i = 0; i < size; ++i) out->push_back(0x00);
        }
        // 车牌颜色及标识.
        out->push_back(register_info.car_plate_color);
        if (register_info.car_plate_color != VehiclePlateColor::kVin) {
          for (auto& ch: register_info.car_plate_num) out->push_back(ch);
          msg_len += register_info.car_plate_num.size();
        }
        return msg_len;
      }
  ));
  // 0x8100, 终端注册应答.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kTerminalRegisterResponse,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 3;
        U16ToU8Array u16converter;
        // 应答消息流水号.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_flow_num);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 应答结果.
        out->push_back(para.respone_result);
        // 应答结果为0(成功)时附加鉴权码.
        if (para.respone_result == 0) {
          for (auto& ch: para.authentication_code) out->push_back(ch);
          msg_len += para.authentication_code.size();
        }
        return msg_len;
      }
  ));
  // 0x0003, 终端注销.
  packager->insert(std::pair<uint16_t, PackageHandler>(kTerminalLogOut,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x0102, 终端鉴权.
  packager->insert(std::pair<uint16_t, PackageHandler>(kTerminalAuthentication,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = para.parse.authentication_code.size();
        // 鉴权码.
        for (auto& ch: para.parse.authentication_code) out->push_back(ch);
        return msg_len;
      }
  ));
  // 0x8103, 设置终端参数.
  packager->insert(std::pair<uint16_t, PackageHandler>(kSetTerminalParameters,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 0;
        uint8_t cnt = static_cast<uint8_t>(para.terminal_parameters.size());
        // 参数项总数.
        out->push_back(cnt);
        ++msg_len;
        U32ToU8Array u32converter;
        // 参数项.
        for (auto const& item : para.terminal_parameters) {
          // 参数ID.
          u32converter.u32val = EndianSwap32(item.first);
          for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
          // 参数长度.
          cnt = static_cast<uint8_t>(item.second.size());
          out->push_back(cnt);
          // 参数值.
          for (auto const& uch : item.second) out->push_back(uch);
          msg_len += 5 + cnt;
        }
        return msg_len;
      }
  ));
  // 0x8104, 查询终端参数.
  packager->insert(std::pair<uint16_t, PackageHandler>(kGetTerminalParameters,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x8106, 查询指定终端参数.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kGetSpecificTerminalParameters,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 0;
        uint8_t cnt = static_cast<uint8_t>(para.terminal_parameter_ids.size());
        // 查询的参数ID总数.
        out->push_back(cnt);
        ++msg_len;
        U32ToU8Array u32converter;
        // 查询的参数ID.
        for (auto const& id : para.terminal_parameter_ids) {
          u32converter.u32val = EndianSwap32(id);
          for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
          msg_len += 4;
        }
        return msg_len;
      }
  ));
  // 0x0104, 查询终端参数应答.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kGetTerminalParametersResponse,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 0;
        U16ToU8Array u16converter;
        // 应答流水号.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_flow_num);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        msg_len += 2;
        if (para.terminal_parameter_ids.empty()) {  // 所有终端参数.
          // 以下与设置终端参数命令消息体内一致.
          uint8_t cnt = static_cast<uint8_t>(para.terminal_parameters.size());
          // 参数项总数.
          out->push_back(cnt);
          ++msg_len;
          U32ToU8Array u32converter;
          // 参数项.
          for (auto const& item : para.terminal_parameters) {
            // 参数ID.
            u32converter.u32val = EndianSwap32(item.first);
            for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
            // 参数长度.
            cnt = static_cast<uint8_t>(item.second.size());
            out->push_back(cnt);
            // 参数值.
            for (auto const& uch : item.second) out->push_back(uch);
            msg_len += 5 + cnt;
          }
        } else {  // 指定终端参数.
          uint8_t cnt = static_cast<uint8_t>(para.terminal_parameter_ids.size());
          // 参数项总数.
          auto pos = static_cast<uint8_t>(out->size());  // 记录总数位置.
          out->push_back(cnt);
          ++msg_len;
          U32ToU8Array u32converter;
          // 参数项.
          for (auto const& id : para.terminal_parameter_ids) {
            auto const& it = para.terminal_parameters.find(id);
            if (it == para.terminal_parameters.end()) {
              --(*out)[pos];  // 未找到终端参数时修正参数项总数.
              continue;
            }
            // 参数ID.
            u32converter.u32val = EndianSwap32(it->first);
            for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
            // 参数长度.
            cnt = static_cast<uint8_t>(it->second.size());
            out->push_back(cnt);
            // 参数值.
            for (auto const& uch : it->second) out->push_back(uch);
            msg_len += 5 + cnt;
          }
        }
        return msg_len;
      }
  ));
  // 0x8108, 下发终端升级包.
  packager->insert(std::pair<uint16_t, PackageHandler>(kTerminalUpgrade,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 11;
        // 升级类型.
        out->push_back(para.upgrade_info.upgrade_type);
        // 制造商ID.
        for (auto const& uch : para.upgrade_info.manufacturer_id) {
          out->push_back(uch);
        }
        // 版本号长度.
        out->push_back(para.upgrade_info.version_id.size());
        // 版本号.
        for (auto const& ch : para.upgrade_info.version_id) {
          out->push_back(static_cast<uint8_t>(ch));
        }
        msg_len += para.upgrade_info.version_id.size();
        // 升级数据包长度.
        uint32_t content_len = para.upgrade_info.upgrade_data_total_len;
        U32ToU8Array u32convert;
        u32convert.u32val = EndianSwap32(content_len);
        for (int i = 0; i < 4; ++i) out->push_back(u32convert.u8array[i]);
        // 升级数据包.
        out->insert(out->end(), para.upgrade_info.upgrade_data.begin(),
                    para.upgrade_info.upgrade_data.end());
        msg_len += para.upgrade_info.upgrade_data.size();
        return msg_len;
      }
  ));
  // 0x0108, 终端升级结果通知.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kTerminalUpgradeResultReport,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 2;
        // 升级类型.
        out->push_back(para.upgrade_info.upgrade_type);
        // 升级结果.
        out->push_back(para.upgrade_info.upgrade_result);
        return msg_len;
      }
  ));
  // 0x0200, 位置信息汇报.
  packager->insert(std::pair<uint16_t, PackageHandler>(kLocationReport,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 28;
        auto& basic_info = para.location_info;
        auto& extension_info = para.location_extension;
        U32ToU8Array u32converter;
        // 报警标志.
        u32converter.u32val = EndianSwap32(basic_info.alarm.value);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 状态.
        u32converter.u32val = EndianSwap32(basic_info.status.value);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 纬度.
        u32converter.u32val = EndianSwap32(basic_info.latitude);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 经度.
        u32converter.u32val = EndianSwap32(basic_info.longitude);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        U16ToU8Array u16converter;
        // 海拔高程.
        u16converter.u16val = EndianSwap16(basic_info.altitude);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 速度.
        u16converter.u16val = EndianSwap16(basic_info.speed);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 方向.
        u16converter.u16val = EndianSwap16(basic_info.bearing);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        std::vector<uint8_t> bcd;
        // UTC时间(BCD-8421码).
        StringToBcd(basic_info.time, &bcd);
        for (auto const& uch : bcd)  out->push_back(uch);
        std::vector<uint8_t> extension_custom;
        // 位置附加信息项.
        for (auto const& item : extension_info) {
          if (item.first <= kCustomInformationLength) {
            out->push_back(item.first);
            if (item.first == kCustomInformationLength) continue;
            out->push_back(item.second.size());
            msg_len += 2 + item.second.size();
            for (auto const& uch : item.second) out->push_back(uch);
          } else if (item.first > kCustomInformationLength) {
            extension_custom.push_back(item.first);
            extension_custom.push_back(item.second.size());
            for (auto const& uch : item.second) extension_custom.push_back(uch);
          }
        }
        auto const& length = extension_custom.size();
        if (length >= 256) {
          out->push_back(2);
          out->push_back(length%65536/256);
          out->push_back(length%256);
          msg_len += 4;
        } else if (length > 0) {
          out->push_back(1);
          out->push_back(length%256);
          msg_len += 3;
        } else {  // 没有后续自定义信息.
          out->pop_back();
        }
        for (auto const& uch : extension_custom) out->push_back(uch);
        msg_len += length;
        return msg_len;
      }
  ));
  // 0x8201, 位置信息查询.
  packager->insert(std::pair<uint16_t, PackageHandler>(kGetLocationInformation,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x0201, 位置信息查询应答.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kGetLocationInformationResponse,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 30;
        U16ToU8Array u16converter;
        // 应答消息流水号.
        u16converter.u16val = EndianSwap16(para.parse.msg_head.msg_flow_num);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 以下为位置信息汇报内容.
        auto& basic_info = para.location_info;
        auto& extension_info = para.location_extension;
        U32ToU8Array u32converter;
        // 报警标志.
        u32converter.u32val = EndianSwap32(basic_info.alarm.value);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 状态.
        u32converter.u32val = EndianSwap32(basic_info.status.value);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 纬度.
        u32converter.u32val = EndianSwap32(basic_info.latitude);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 经度.
        u32converter.u32val = EndianSwap32(basic_info.longitude);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        // 海拔高程.
        u16converter.u16val = EndianSwap16(basic_info.altitude);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 速度.
        u16converter.u16val = EndianSwap16(basic_info.speed);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 方向.
        u16converter.u16val = EndianSwap16(basic_info.bearing);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        std::vector<uint8_t> bcd;
        // UTC时间(BCD-8421码).
        StringToBcd(basic_info.time, &bcd);
        for (auto const& uch : bcd)  out->push_back(uch);
        std::vector<uint8_t> extension_custom;
        // 位置附加信息项.
        for (auto const& item : extension_info) {
          if (item.first <= kCustomInformationLength) {
            out->push_back(item.first);
            if (item.first == kCustomInformationLength) continue;
            out->push_back(item.second.size());
            msg_len += 2 + item.second.size();
            for (auto const& uch : item.second) out->push_back(uch);
          } else if (item.first > kCustomInformationLength) {
            extension_custom.push_back(item.first);
            extension_custom.push_back(item.second.size());
            for (auto const& uch : item.second) extension_custom.push_back(uch);
          }
        }
        auto const& length = extension_custom.size();
        if (length >= 256) {
          out->push_back(2);
          out->push_back(length%65536/256);
          out->push_back(length%256);
          msg_len += 4;
        } else if (length > 0) {
          out->push_back(1);
          out->push_back(length%256);
          msg_len += 3;
        } else {  // 没有后续自定义信息.
          out->pop_back();
        }
        for (auto const& uch : extension_custom) out->push_back(uch);
        msg_len += length;
        return msg_len;
      }
  ));
  // 0x8202, 临时位置跟踪控制.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kLocationTrackingControl,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 6;
        // 跟踪期间位置信息汇报时间间隔.
        U16ToU8Array u16converter;
        u16converter.u16val =
            EndianSwap16(para.location_tracking_control.interval);
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[i]);
        // 跟踪有效时间.
        U32ToU8Array u32converter;
        u32converter.u32val =
            EndianSwap32(para.location_tracking_control.tracking_time);
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
        return msg_len;
      }
  ));
  // 0x8604, 设置多边形区域.
  packager->insert(std::pair<uint16_t, PackageHandler>(kSetPolygonArea,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        auto const& polygon_area = para.polygon_area;
        int msg_len = 6;
        U16ToU8Array u16converter;
        U32ToU8Array u32converter;
        // 区域ID.
        u32converter.u32val = polygon_area.area_id;
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[3-i]);
        // 区域属性.
        u16converter.u16val = polygon_area.area_attribute.value;
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[1-i]);
        // 起始时间, 在区域属性中相关标志位为1时才启用.
        if (polygon_area.area_attribute.bit.by_time) {
          std::vector<uint8_t> bcd;
          StringToBcd(polygon_area.start_time, &bcd);
          for (auto const& uch : bcd)  out->push_back(uch);
          msg_len += polygon_area.start_time.size();
          StringToBcd(polygon_area.stop_time, &bcd);
          for (auto const& uch : bcd)  out->push_back(uch);
          msg_len += polygon_area.stop_time.size();
        }
        // 限速, 在区域属性中相关标志位为1时才启用.
        if (polygon_area.area_attribute.bit.speed_limit) {
          u16converter.u16val = polygon_area.max_speed;
          for (int i = 0; i < 2; ++i)
            out->push_back(u16converter.u8array[1-i]);
          out->push_back(polygon_area.overspeed_time);
          msg_len += 3;
        }
        // 顶点个数.
        u16converter.u16val = polygon_area.vertices.size();
        for (int i = 0; i < 2; ++i) out->push_back(u16converter.u8array[1-i]);
        msg_len += 2;
        // 所有顶点经纬度.
        for (auto const& vertex: polygon_area.vertices) {
          // 纬度.
          u32converter.u32val = static_cast<uint32_t>(vertex.latitude*1e6);
          for (int i = 0; i < 4; ++i)
            out->push_back(u32converter.u8array[3-i]);
          // 经度.
          u32converter.u32val = static_cast<uint32_t>(vertex.longitude*1e6);
          for (int i = 0; i < 4; ++i)
            out->push_back(u32converter.u8array[3-i]);
          msg_len += 8;
        }
        return msg_len;
      }
  ));
  // 0x8605, 删除多边形区域.
  packager->insert(std::pair<uint16_t, PackageHandler>(kDeletePolygonArea,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        auto const& polygon_area_id = para.polygon_area_id;
        if (polygon_area_id.empty()) return -1;
        // 删除区域ID总个数.
        out->push_back(polygon_area_id.size());
        int msg_len = 1+polygon_area_id.size()*4;
        U32ToU8Array u32converter;
        // 需要删除的所有区域ID.
        for (auto const& id: polygon_area_id) {
            u32converter.u32val = id;
            for (int i = 0; i < 4; ++i)
              out->push_back(u32converter.u8array[3-i]);
        }
        return msg_len;
      }
  ));
  // 0x0801, 多媒体数据上传.
  packager->insert(std::pair<uint16_t, PackageHandler>(kMultimediaDataUpload,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 36 + para.multimedia_upload.media_data.size();
        U32ToU8Array u32converter;
        u32converter.u32val = para.multimedia_upload.media_id;
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[1-i]);
        out->push_back(para.multimedia_upload.media_type);
        out->push_back(para.multimedia_upload.media_format);
        out->push_back(para.multimedia_upload.media_event);
        out->push_back(para.multimedia_upload.channel_id);
        out->insert(out->end(),
                    para.multimedia_upload.loaction_report_body.begin(),
                    para.multimedia_upload.loaction_report_body.end());
        out->insert(out->end(), para.multimedia_upload.media_data.begin(),
                    para.multimedia_upload.media_data.end());
        return msg_len;
      }));
  // 0x8800, 多媒体数据上传应答.
  packager->insert(std::pair<uint16_t, PackageHandler>(
      kMultimediaDataUploadResponse,
      [] (ProtocolParameter const& para, std::vector<uint8_t>* out) {
        if (out == nullptr) return -1;
        int msg_len = 4;
        U32ToU8Array u32converter;
        U16ToU8Array u16converter;
        u32converter.u32val = para.multimedia_upload_response.media_id;
        for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[1-i]);
        if (!para.multimedia_upload_response.reload_packet_ids.empty()) {
          auto const& ids = para.multimedia_upload_response.reload_packet_ids;
          out->push_back(ids.size());
          for (auto& id : ids) {
            u16converter.u16val = id;
            for (int i = 0; i < 2; ++i) {
              out->push_back(u16converter.u8array[1-i]);
            }
            msg_len += 2;
          }
        } 
        return msg_len;
      }));
  return 0;
}

// 额外增加封装器支持命令.
bool JT808FramePackagerAppend(
    Packager* packager, std::pair<uint16_t, PackageHandler> const& pair) {
  if (packager == nullptr) return false;
  return packager->insert(pair).second;
}

// 额外增加封装器支持命令.
bool JT808FramePackagerAppend(Packager* packager,
                              uint16_t const& msg_id,
                              PackageHandler const& handler) {
  return JT808FramePackagerAppend(packager, {msg_id, handler});
}

// 重写封装器支持命令.
bool JT808FramePackagerOverride(
    Packager* packager, std::pair<uint16_t, PackageHandler> const& pair) {
  if (packager == nullptr) return false;
  for (auto const& item : *packager) {
    if (item.first == pair.first) {
      packager->erase(item.first);
      break;
    }
  }
  return packager->insert(pair).second;
}

// 重写封装器支持命令.
bool JT808FramePackagerOverride(Packager* packager,
                                uint16_t const& msg_id,
                                PackageHandler const& handler) {
  return JT808FramePackagerOverride(packager, {msg_id, handler});
}

// 封装命令.
int JT808FramePackage(Packager const& packager,
                      ProtocolParameter const& para,
                      std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  auto it = packager.find(para.msg_head.msg_id);
  if (it == packager.end()) return -1;
  out->clear();
  // 生成消息头
  if (JT808FrameHeadPackage(para.msg_head, out) < 0) return -1;
  // 封装消息内容.
  int ret = it->second(para, out);
  if (ret >= 0) {
    // 修正消息长度.
    if (JT808MsgBodyLengthFix(para.msg_head, ret, out) < 0) return -1;
    // 校验码.
    out->push_back(BccCheckSum(&((*out)[1]), out->size()-1));
    // 结束标识位.
    out->push_back(PROTOCOL_SIGN);
    // 处理转义.
    if (JT808MsgEscape(out) < 0) return -1;
    return 0;
  }
  return -1;
}

}  // namespace libjt808
