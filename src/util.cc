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

// @File    :  util.cc
// @Version :  1.0
// @Time    :  2020/06/24 10:04:23
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "util.h"

#include "protocol_parameter.h"


namespace libjt808 {

// 转义函数.
int Escape(std::vector<uint8_t> const& in,
           std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  out->clear();
  for (auto& u8val: in) {
    if (u8val == PROTOCOL_SIGN) {
      out->push_back(PROTOCOL_ESCAPE);
      out->push_back(PROTOCOL_ESCAPE_SIGN);
    } else if (u8val == PROTOCOL_ESCAPE) {
      out->push_back(PROTOCOL_ESCAPE);
      out->push_back(PROTOCOL_ESCAPE_ESCAPE);
    } else {
      out->push_back(u8val);
    }
  }
  return 0;
}

// 逆转义函数.
int ReverseEscape(std::vector<uint8_t> const& in,
                  std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  out->clear();
  for (size_t i = 0; i < in.size(); ++i) {
    if ((in[i] == PROTOCOL_ESCAPE) && (in[i+1] == PROTOCOL_ESCAPE_SIGN)) {
      out->push_back(PROTOCOL_SIGN);
      ++i;
    } else if ((in[i] == PROTOCOL_ESCAPE) &&
               (in[i+1] == PROTOCOL_ESCAPE_ESCAPE)) {
      out->push_back(PROTOCOL_ESCAPE);
      ++i;
    } else {
      out->push_back(in[i]);
    }
  }
  return 0;
}

// 奇偶校验.
uint8_t BccCheckSum(const uint8_t *src, const size_t &len) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < len; ++i) {
    checksum = checksum ^ src[i];
  }
  return checksum;
}

}  // namespace libjt808
