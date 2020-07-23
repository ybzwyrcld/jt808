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

// @File    :  location_report.cc
// @Version :  1.0
// @Time    :  2020/07/09 14:35:15
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "location_report.h"

#include <string.h>

#include "util.h"


namespace libjt808 {

// 设置超速报警报警附加项消息体.
int SetOverSpeedAlarmBody(uint8_t const& location_type,
                          uint32_t const& area_route_id,
                          std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  out->clear();
  out->push_back(location_type);
  if (location_type != kOverSpeedAlarmNoSpecificLocation) {
    U32ToU8Array u32converter;
    u32converter.u32val = libjt808::EndianSwap32(area_route_id);
    for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
  }
  return 0;
}

// 获得超速报警报警附加信息消息体.
int GetOverSpeedAlarmBody(std::vector<uint8_t> const& out,
                          uint8_t* location_type,
                          uint32_t* area_route_id) {
  if (location_type == nullptr || area_route_id == nullptr) return -1;
  if (out.size() != 1 || out.size() != 5) return -1;
  *location_type = out[0];
  if (*location_type != kOverSpeedAlarmNoSpecificLocation) {
    if (out.size() != 5) return -1;
  }
  U32ToU8Array u32converter;
  memcpy(u32converter.u8array, &(out[1]), 4);
  *area_route_id = EndianSwap32(u32converter.u32val);
  return 0;
}

// 设置进出区域/路线报警附加项消息体.
int SetAccessAreaAlarmBody(uint8_t const& location_type,
                           uint32_t const& area_route_id,
                           uint8_t const& direction,
                           std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  out->clear();
  out->push_back(location_type);
  U32ToU8Array u32converter;
  u32converter.u32val = libjt808::EndianSwap32(area_route_id);
  for (int i = 0; i < 4; ++i) out->push_back(u32converter.u8array[i]);
  out->push_back(direction);
  return 0;
}

// 获得进出区域/路线报警附加信息消息体.
int GetAccessAreaAlarmBody(std::vector<uint8_t> const& out,
                           uint8_t* location_type,
                           uint32_t* area_route_id,
                           uint8_t* direction) {
  if (location_type == nullptr ||
      area_route_id == nullptr ||
      direction == nullptr ||
      out.size() != 6) return -1;
  *location_type = out[0];
  U32ToU8Array u32converter;
  memcpy(u32converter.u8array, &(out[1]), 4);
  *area_route_id = EndianSwap32(u32converter.u32val);
  *direction = out[5];
  return 0;
}

}  // namespace libjt808
