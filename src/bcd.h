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

// @File    :  bcd.h
// @Version :  1.0
// @Time    :  2020/06/24 14:56:32
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_BCD_H_
#define JT808_BCD_H_

#include <stdint.h>

#include <string>
#include <vector>

namespace libjt808 {

uint8_t HexToBcd(uint8_t const& src);
uint8_t BcdToHex(uint8_t const&src);
uint8_t* StringToBcdCompress(const uint8_t *src, uint8_t *dst, const int &srclen);
uint8_t* BcdToStringCompress(const uint8_t *src, uint8_t *dst, const int &srclen);
uint8_t* BcdToStringCompressFillingZero(const uint8_t *src,
                                        uint8_t *dst, const int &srclen);
int StringToBcd(std::string const& in, std::vector<uint8_t>* out);
int BcdToString(std::vector<uint8_t> const& in, std::string* out);
int BcdToStringFillZero(std::vector<uint8_t> const& in, std::string* out);
}  // namespace libjt808

#endif  // JT808_BCD_H_
