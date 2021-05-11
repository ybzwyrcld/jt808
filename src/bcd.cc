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

// @File    :  bcd.cc
// @Version :  1.0
// @Time    :  2020/06/24 14:49:18
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#include "jt808/bcd.h"


namespace libjt808 {

uint8_t HexToBcd(uint8_t const& src) {
  uint8_t temp;
  temp = ((src / 10) << 4) + (src % 10);
  return temp;
}

uint8_t BcdToHex(uint8_t const&src) {
  uint8_t temp;
  temp = (src >> 4)*10 + (src & 0x0f);
  return temp;
}

uint8_t *StringToBcdCompress(const uint8_t *src, uint8_t *dst, const int &srclen) {
  uint8_t *ptr = dst;
  uint8_t temp;
  if (srclen % 2 != 0) {
    *ptr++ = HexToBcd(*src++-'0');
  }
  while (*src) {
    temp = *src++ - '0';
    temp *= 10;
    temp += *src++ - '0';
    *ptr++ = HexToBcd(temp);
  }
  *ptr = 0;
  return dst;
}

uint8_t *BcdToStringCompress(const uint8_t *src, uint8_t *dst, const int &srclen) {
  uint8_t *ptr = dst;
  uint8_t temp;
  int cnt = srclen;
  while (cnt--) {
    temp = BcdToHex(*src);
    *ptr++ = temp/10 + '0';
    if (dst[0] == '0') {
      ptr = dst;
    }
    *ptr++ = temp%10 + '0';
    ++src;
  }
  return dst;
}

uint8_t *BcdToStringCompressFillingZero(const uint8_t *src,
                                        uint8_t *dst, const int &srclen) {
  uint8_t *ptr = dst;
  uint8_t temp;
  int cnt = srclen;
  while (cnt--) {
    temp = BcdToHex(*src);
    *ptr++ = temp/10 + '0';
    *ptr++ = temp%10 + '0';
    ++src;
  }
  return dst;
}

int StringToBcd(std::string const& in, std::vector<uint8_t>* out) {
  if (out == nullptr) return -1;
  out->clear();
  size_t pos = 0;
  if (in.size() % 2 != 0) {
    out->push_back(HexToBcd(in[pos]-'0'));
    ++pos;
  }
  uint8_t tmp = 0;
  for (; pos < in.size();) {
    tmp = (in[pos]-'0')*10 + (in[pos+1]-'0');
    out->push_back(HexToBcd(tmp));
    pos += 2;
  }
  return 0;
}

int BcdToString(std::vector<uint8_t> const& in, std::string* out) {
  if (out == nullptr) return -1;
  out->clear();
  size_t pos = 0;
  uint8_t tmp = BcdToHex(in[pos]);
  if (tmp / 10 == 0) {
    out->push_back(tmp%10+'0');
    ++pos;
  }
  for (; pos < in.size(); ++pos) {
    tmp = BcdToHex(in[pos]);
    out->push_back(tmp/10+'0');
    out->push_back(tmp%10+'0');
  }
  return 0;
}

int BcdToStringFillZero(std::vector<uint8_t> const& in, std::string* out) {
  if (out == nullptr) return -1;
  out->clear();
  uint8_t tmp = 0;
  for (auto& uch : in) {
    tmp = BcdToHex(uch);
    out->push_back(tmp/10+'0');
    out->push_back(tmp%10+'0');
  }
  return 0;
}

}  // namespace libjt808
