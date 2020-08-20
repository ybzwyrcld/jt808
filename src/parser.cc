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

// @File    :  parser.cc
// @Version :  1.0
// @Time    :  2020/07/08 17:09:29
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "parser.h"

#include <string.h>

#include "bcd.h"
#include "util.h"


namespace libjt808 {

namespace {

// 解析消息头.
int JT808FrameHeadParse(std::vector<uint8_t> const& in,
                        MsgHead* msg_head) {
  if (msg_head == nullptr || in.size() < 15) return -1;
  // 消息ID.
  msg_head->msg_id = in[1]*256 + in[2];
  // 消息体属性.
  msg_head->msgbody_attr.u16val = in[3]*256 + in[4];
  // 终端手机号.
  std::vector<uint8_t> phone_num_bcd;
  phone_num_bcd.assign(in.begin()+5,in.begin()+11);
  if (BcdToString(phone_num_bcd, &(msg_head->phone_num)) != 0) return -1;
  // 消息流水号.
  msg_head->msg_flow_num =in[11]*256 + in[12];
  // 出现封包.
  if ((msg_head->msgbody_attr.bit.packet == 1) &&
      ((in.size()-15-msg_head->msgbody_attr.bit.msglen) == 4)) {
    msg_head->total_packet = in[13]*256 + in[14];
    msg_head->packet_seq = in[15]*256 + in[16];
  } else {
     msg_head->total_packet = 0;
     msg_head->packet_seq = 0;
  }
  return 0;
}

}  // namespace

// 命令解析器初始化.
int JT808FrameParserInit(Parser* parser) {
  // 0x0001, 终端通用应答.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalGeneralResponse,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        // if (para->msg_head.msgbody_attr.bit.package == 1)
        //   pos = MSGBODY_PACKET_POS;
        // 应答流水号.
        para->parse.respone_flow_num = in[pos]*256 + in[pos+1];
        // 应答消息ID.
        para->parse.respone_msg_id = in[pos+2]*256 + in[pos+3];
        // 应答结果.
        para->parse.respone_result = in[pos+4];
        return 0;
      }
  ));
  // 0x8001, 平台通用应答.
  parser->insert(std::pair<uint16_t, ParseHandler>(kPlatformGeneralResponse,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        // if (para->msg_head.msgbody_attr.bit.package == 1)
        //   pos = MSGBODY_PACKET_POS;
        // 应答流水号.
        para->parse.respone_flow_num = in[pos]*256 + in[pos+1];
        // 应答消息ID.
        para->parse.respone_msg_id = in[pos+2]*256 + in[pos+3];
        // 应答结果.
        para->parse.respone_result = in[pos+4];
        return 0;
      }
  ));
  // 0x0002, 终端心跳.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalHeartBeat,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x8003, 补传分包请求.
  parser->insert(std::pair<uint16_t, ParseHandler>(kFillPacketRequest,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto& fill_packet = para->parse.fill_packet;
        // 首包流水号.
        fill_packet.first_packet_msg_flow_num = in[pos]*256 + in[pos+1];
        pos += 2;
        // 重传包总数.
        uint16_t cnt = in[pos];
        ++pos;
        // 重传包ID.
        if (msg_len-3 != cnt*2) return -1;
        fill_packet.packet_id.clear();
        uint16_t id = 0;
        for (uint8_t i = 0; i < cnt; ++i) {
          id = in[pos+i*2] + in[pos+1+i*2];
          fill_packet.packet_id.push_back(id);
        }
       return 0;
      }
  ));
  // 0x0100, 终端注册.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalRegister,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        auto& register_info = para->parse.register_info;
        // 省域ID.
        U16ToU8Array u16converter;
        for (int i = 0; i < 2; ++i) u16converter.u8array[i] = in[pos++];
        register_info.province_id = EndianSwap16(u16converter.u16val);
        // 市县域ID.
        for (int i = 0; i < 2; ++i) u16converter.u8array[i] = in[pos++];
        register_info.city_id = EndianSwap16(u16converter.u16val);
        // 制造商ID.
        register_info.manufacturer_id.clear();
        for (size_t i = 0; i < 5; ++i)
          register_info.manufacturer_id.push_back(in[pos+i]);
        pos += 5;
        // 终端型号.
        for (size_t i = 0; i < 20; ++i) {
          if (in[pos+i] == 0x0) break;
          register_info.terminal_model.push_back(in[pos+i]);
        }
        pos += 20;
        // 终端ID.
        for (size_t i = 0; i < 7; ++i) {
          if (in[pos+i] == 0x0) break;
          register_info.terminal_id.push_back(in[pos+i]);
        }
        pos += 7;
        // 车牌颜色及标识.
        register_info.car_plate_color = in[pos++];
        if (register_info.car_plate_color != VehiclePlateColor::kVin) {
          register_info.car_plate_num.clear();
          size_t len = para->parse.msg_head.msgbody_attr.bit.msglen-37;
          register_info.car_plate_num.assign(
              in.begin()+pos, in.begin()+pos+len);
        }
        return 0;
      }
  ));
  // 0x8100, 终端注册应答.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalRegisterResponse,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        // 应答流水号.
        para->parse.respone_flow_num = in[pos]*256 + in[pos+1];
        // 应答结果.
        para->parse.respone_result = in[pos+2];
        // 应答结果为0(成功)时解析出附加的鉴权码.
        if (para->parse.respone_result == 0) {
          auto begin = in.begin()+pos+3;
          auto end = begin + para->parse.msg_head.msgbody_attr.bit.msglen-3;
          para->parse.authentication_code.clear();
          para->parse.authentication_code.assign(begin, end);
        }
        return 0;
      }
  ));
  // 0x0003, 终端注销.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalLogOut,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x0102, 终端鉴权.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalAuthentication,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        // 提取鉴权码.
        auto begin = in.begin() + pos;
        auto end = begin + para->parse.msg_head.msgbody_attr.bit.msglen;
        para->parse.authentication_code.clear();
        para->parse.authentication_code.assign(begin, end);
        return 0;
      }
  ));
  // 0x8103, 设置终端参数.
  parser->insert(std::pair<uint16_t, ParseHandler>(kSetTerminalParameters,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len < 1) return -1;
        // 设置的参数总个数.
        uint8_t cnt = in[pos];
        ++pos;
        U32ToU8Array u32converter;
        // 设置的参数项.
        uint32_t id = 0;
        std::vector<uint8_t> value;
        auto& paras = para->parse.terminal_parameters;
        for (int i = 0; i < cnt; ++i) {
          // 参数ID.
          memcpy(u32converter.u8array, &(in[pos]), 4);
          id = EndianSwap32(u32converter.u32val);
          pos += 4;
          // 参数值.
          value.clear();
          value.assign(in.begin()+pos+1, in.begin()+pos+1+in[pos]);
          paras.insert(std::make_pair(id, value));
          pos += 1 + in[pos];
        }
        return 0;
      }
  ));
  // 0x8104, 查询终端参数.
  parser->insert(std::pair<uint16_t, ParseHandler>(kGetTerminalParameters,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        // 用来区分是否为查询特殊终端参数.
        para->parse.terminal_parameter_ids.clear();
        // 空消息体.
        return 0;
      }
  ));
  // 0x8106, 查询指定终端参数.
  parser->insert(std::pair<uint16_t, ParseHandler>(
      kGetSpecificTerminalParameters,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len < 1) return -1;
        // 参数ID总数.
        uint8_t cnt = in[pos++];
        if (msg_len != cnt *4 + 1) return -1;
        // 参数ID解析.
        uint32_t id = 0;
        U32ToU8Array u32converter;
        para->parse.terminal_parameter_ids.clear();
        for (uint8_t i = 0; i < cnt; ++i) {
          memcpy(u32converter.u8array, &(in[pos]), 4);
          id = EndianSwap32(u32converter.u32val);
          para->parse.terminal_parameter_ids.push_back(id);
          pos += 4;
        }
        return 0;
      }
  ));
  // 0x0104, 查询终端参数应答.
  parser->insert(std::pair<uint16_t, ParseHandler>(
      kGetTerminalParametersResponse,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len < 3) return -1;
        // 应答流水号.
        U16ToU8Array u16converter;
        memcpy(u16converter.u8array, &(in[pos]), 2);
        para->parse.respone_flow_num = EndianSwap16(u16converter.u16val);
        pos += 2;
        // 后续内容与设置终端参数解析一致.
        // 设置的参数总个数.
        uint8_t cnt = in[pos];
        ++pos;
        U32ToU8Array u32converter;
        // 参数项.
        uint32_t id = 0;
        std::vector<uint8_t> value;
        auto& paras = para->parse.terminal_parameters;
        for (int i = 0; i < cnt; ++i) {
          // 参数ID.
          memcpy(u32converter.u8array, &(in[pos]), 4);
          id = EndianSwap32(u32converter.u32val);
          pos += 4;
          // 参数值.
          value.clear();
          value.assign(in.begin()+pos+1, in.begin()+pos+1+in[pos]);
          paras.insert(std::make_pair(id, value));
          pos += 1 + in[pos];
        }
        return 0;
      }
  ));
  // 0x8108, 下发终端升级包.
  parser->insert(std::pair<uint16_t, ParseHandler>(kTerminalUpgrade,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto& upgrade_info = para->parse.upgrade_info;
        // 升级类型.
        upgrade_info.upgrade_type = in[pos++];
        // 制造商ID.
        upgrade_info.manufacturer_id.clear();
        for (int i = 0; i < 5; ++i) {
          upgrade_info.manufacturer_id.push_back(in[pos++]);
        }
        // 升级版本号.
        upgrade_info.version_id.clear();
        for (int i = 0; i < in[pos]; ++i) {
          upgrade_info.version_id.push_back(static_cast<char>(in[pos+i+1]));
        }
        pos += in[pos] + 1;
        // 升级数据包内容.
        uint16_t content_len = in[pos]*256+in[pos+1];
        pos += 2;
        if (content_len+9+upgrade_info.version_id.size() > msg_len) return -1;
        upgrade_info.upgrade_data.clear();
        upgrade_info.upgrade_data.assign(in.begin()+pos, in.begin()+pos+content_len);
        return 0;
      }
  ));
  // 0x0108, 终端升级结果通知.
  parser->insert(std::pair<uint16_t, ParseHandler>(
      kTerminalUpgradeResultReport,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto& upgrade_info = para->parse.upgrade_info;
        // 升级类型.
        upgrade_info.upgrade_type = in[pos++];
        // 升级结果.
        upgrade_info.upgrade_result = in[pos++];
       return 0;
      }
  ));
  // 0x0200, 位置信息汇报.
  parser->insert(std::pair<uint16_t, ParseHandler>(kLocationReport,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len < 28) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto& basic_info = para->parse.location_info;
        auto& extension_info = para->parse.location_extension;
        U32ToU8Array u32converter;
        // 报警标志.
        memcpy(u32converter.u8array, &(in[pos]), 4);
        basic_info.alarm.value = EndianSwap32(u32converter.u32val);
        // 状态.
        memcpy(u32converter.u8array, &(in[pos+4]), 4);
        basic_info.status.value = EndianSwap32(u32converter.u32val);
        // 纬度.
        memcpy(u32converter.u8array, &(in[pos+8]), 4);
        basic_info.latitude = EndianSwap32(u32converter.u32val);
        // 经度.
        memcpy(u32converter.u8array, &(in[pos+12]), 4);
        basic_info.longitude = EndianSwap32(u32converter.u32val);
        U16ToU8Array u16converter;
        // 海拔高程.
        memcpy(u16converter.u8array, &(in[pos+16]), 2);
        basic_info.altitude = EndianSwap16(u16converter.u16val);
        // 速度.
        memcpy(u16converter.u8array, &(in[pos+18]), 2);
        basic_info.speed = EndianSwap16(u16converter.u16val);
        // 方向.
        memcpy(u16converter.u8array, &(in[pos+20]), 2);
        basic_info.bearing = EndianSwap16(u16converter.u16val);
        // UTC时间(BCD-8421码).
        std::vector<uint8_t> bcd;
        bcd.assign(in.begin()+pos+22, in.begin()+pos+28);
        BcdToStringFillZero(bcd, &basic_info.time);
        if (msg_len > 28) {  // 位置附加信息项.
          uint8_t end = msg_len + pos;
          pos += 28;
          std::vector<uint8_t> item_content;
          while (pos <= end-2) {  // 附加信息长度至少为1.
            if (pos+1+in[pos+1] > end) return -1;  // 附加信息长度超出范围.
            item_content.assign(in.begin()+pos+2, in.begin()+pos+2+in[pos+1]);
            extension_info[in[pos]] = item_content;
            pos += 2 + in[pos+1];
          }
        }
        return 0;
      }
  ));
  // 0x8201, 位置信息查询.
  parser->insert(std::pair<uint16_t, ParseHandler>(kGetLocationInformation,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        // 空消息体.
        return 0;
      }
  ));
  // 0x0201, 位置信息查询应答.
  parser->insert(std::pair<uint16_t, ParseHandler>(
      kGetLocationInformationResponse,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len < 30) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        // 应答流水号.
        para->parse.respone_flow_num = in[pos]*256+in[pos];
        pos += 2;
        // 以下为位置信息汇报内容.
        auto& basic_info = para->parse.location_info;
        auto& extension_info = para->parse.location_extension;
        U32ToU8Array u32converter;
        // 报警标志.
        memcpy(u32converter.u8array, &(in[pos]), 4);
        basic_info.alarm.value = EndianSwap32(u32converter.u32val);
        // 状态.
        memcpy(u32converter.u8array, &(in[pos+4]), 4);
        basic_info.status.value = EndianSwap32(u32converter.u32val);
        // 纬度.
        memcpy(u32converter.u8array, &(in[pos+8]), 4);
        basic_info.latitude = EndianSwap32(u32converter.u32val);
        // 经度.
        memcpy(u32converter.u8array, &(in[pos+12]), 4);
        basic_info.longitude = EndianSwap32(u32converter.u32val);
        U16ToU8Array u16converter;
        // 海拔高程.
        memcpy(u16converter.u8array, &(in[pos+16]), 2);
        basic_info.altitude = EndianSwap16(u16converter.u16val);
        // 速度.
        memcpy(u16converter.u8array, &(in[pos+18]), 2);
        basic_info.speed = EndianSwap16(u16converter.u16val);
        // 方向.
        memcpy(u16converter.u8array, &(in[pos+20]), 2);
        basic_info.bearing = EndianSwap16(u16converter.u16val);
        // UTC时间(BCD-8421码).
        std::vector<uint8_t> bcd;
        bcd.assign(in.begin()+pos+22, in.begin()+pos+28);
        BcdToStringFillZero(bcd, &basic_info.time);
        if (msg_len > 28) {  // 位置附加信息项.
          uint8_t end = msg_len + pos;
          pos += 28;
          std::vector<uint8_t> item_content;
          while (pos <= end-2) { // 附加信息长度至少为1.
            if (pos+1+in[pos+1] > end) return -1;  // 附加信息长度超出范围.
            item_content.assign(in.begin()+pos+2, in.begin()+pos+2+in[pos+1]);
            extension_info[in[pos]] = item_content;
            pos += 2 + in[pos+1];
          }
        }
        return 0;
      }
  ));
  // 0x8202, 临时位置跟踪控制.
  parser->insert(std::pair<uint16_t, ParseHandler>(kLocationTrackingControl,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len != 6) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        auto& ctrl = para->parse.location_tracking_control;
        // 跟踪期间位置信息汇报时间间隔.
        ctrl.interval = in[pos]*256 + in[pos+1];
        pos += 2;
        // 跟踪有效时间.
        U32ToU8Array u32converter;
        memcpy(u32converter.u8array, &(in[pos]), 4);
        ctrl.tracking_time = EndianSwap32(u32converter.u32val);
        return 0;
      }
  ));
  // 0x08604, 设置多边形区域.
  parser->insert(std::pair<uint16_t, ParseHandler>(kSetPolygonArea,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        if (msg_len < 28) return -1;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        uint16_t end = pos + msg_len;
        auto& polygon_area = para->parse.polygon_area;
        U16ToU8Array u16converter;
        U32ToU8Array u32converter;
        // 区域ID.
        memcpy(u32converter.u8array, &(in[pos]), 4);
        polygon_area.area_id = EndianSwap32(u32converter.u32val);
        pos += 4;
        // 区域属性.
        memcpy(u16converter.u8array, &(in[pos]), 2);
        polygon_area.area_attribute.value = EndianSwap16(u16converter.u16val);
        pos += 2;
        // 起始时间, 在区域属性中相关标志位为1时才启用.
        if (polygon_area.area_attribute.bit.by_time) {
          std::vector<uint8_t> bcd;
          bcd.assign(in.begin()+pos, in.begin()+pos);
          BcdToStringFillZero(bcd, &polygon_area.start_time);
          pos += 6;
          bcd.clear();
          bcd.assign(in.begin()+pos, in.begin()+pos);
          BcdToStringFillZero(bcd, &polygon_area.stop_time);
          pos += 6;
        }
        // 限速, 在区域属性中相关标志位为1时才启用.
        if (polygon_area.area_attribute.bit.speed_limit) {
          memcpy(u16converter.u8array, &(in[pos]), 2);
          polygon_area.max_speed = EndianSwap16(u16converter.u16val);
          pos += 2;
          polygon_area.overspeed_time = in[pos];
          ++pos;
        }
        ++pos;
        // 检查后续内容长度.
        if (end-pos != in[pos-1]*8) return -1;
        LocationPoint location_point {};
        polygon_area.vertices.clear();
        // 所有顶点经纬度.
        while (pos < end) {
          memcpy(u32converter.u8array, &(in[pos]), 4);
          location_point.latitude = EndianSwap32(u32converter.u32val) * 1e-6;
          pos += 4;
          memcpy(u32converter.u8array, &(in[pos]), 4);
          location_point.longitude = EndianSwap32(u32converter.u32val) * 1e-6;
          pos += 4;
          polygon_area.vertices.push_back(location_point);
        }
        return 0;
      }
  ));
  // 0x08605, 删除多边形区域.
  parser->insert(std::pair<uint16_t, ParseHandler>(kDeletePolygonArea,
      [] (std::vector<uint8_t> const& in, ProtocolParameter* para) -> int {
        if (para == nullptr) return -1;
        auto const& msg_len = para->parse.msg_head.msgbody_attr.bit.msglen;
        uint16_t pos = MSGBODY_NOPACKET_POS;
        if (para->parse.msg_head.msgbody_attr.bit.packet == 1)
          pos = MSGBODY_PACKET_POS;
        // 删除区域个数.
        uint8_t cnt = in[pos];
        if (cnt*4+1 != msg_len) return -1;
        auto& polygon_area_id = para->parse.polygon_area_id;
        polygon_area_id.clear();
        U32ToU8Array u32converter;
        // 需要删除的所有区域ID.
        for (uint8_t i = 0; i < cnt; ++i) {
          memcpy(u32converter.u8array, &(in[pos+1+i*4]), 4);
          polygon_area_id.push_back(EndianSwap32(u32converter.u32val));
        }
        return 0;
      }
  ));
  return 0;
}

// 额外增加解析器支持命令.
bool JT808FrameParserAppend(
    Parser* parser, std::pair<uint16_t, ParseHandler> const& pair) {
  if (parser == nullptr) return false;
  return parser->insert(pair).second;
}

// 额外增加解析器支持命令.
bool JT808FrameParserAppend(Parser* parser,
                            uint16_t const& msg_id,
                            ParseHandler const& handler) {
  if (parser == nullptr) return false;
  return parser->insert(std::make_pair(msg_id, handler)).second;
}

// 重写解析器支持命令.
bool JT808FrameParserOverride(
    Parser* parser, std::pair<uint16_t, ParseHandler> const& pair) {
  if (parser == nullptr) return false;
  for (auto const& item: *parser) {
    if (item.first == pair.first) {
      parser->erase(item.first);
      break;
    }
  }
  return parser->insert(pair).second;
}

// 重写解析器支持命令.
bool JT808FrameParserOverride(Parser* parser,
                              uint16_t const& msg_id,
                              ParseHandler const& handler) {
  if (parser == nullptr) return false;
  for (auto const& item: *parser) {
    if (item.first == msg_id) {
      parser->erase(item.first);
      break;
    }
  }
  return parser->insert(std::make_pair(msg_id, handler)).second;
}

// 解析命令.
int JT808FrameParse(Parser const& parser,
                    std::vector<uint8_t> const& in,
                    ProtocolParameter* para) {
  if (para == nullptr) return -1;
  std::vector<uint8_t> out;
  // 逆转义.
  if (ReverseEscape(in, &out) < 0) return -1;
  // 异或校验检查.
  if (BccCheckSum(&(out[1]), out.size()-3) != *(out.end()-2)) return -1;
  // 解析消息头.
  if (JT808FrameHeadParse(out, &para->parse.msg_head) != 0) return -1;
  para->msg_head.phone_num = para->parse.msg_head.phone_num;
  // 解析消息内容.
  auto it = parser.find(para->parse.msg_head.msg_id);
  if (it == parser.end()) return -1;
  return it->second(out, para);
}

}  // namespace libjt808
