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

// @File    :  packager.h
// @Version :  1.0
// @Time    :  2020/07/08 16:52:33
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_PACKAGER_H_
#define JT808_PACKAGER_H_

#include <stdint.h>

#include <functional>
#include <map>
#include <utility>
#include <vector>

#include "jt808/protocol_parameter.h"

namespace libjt808 {

// 消息体封装函数定义.
// 成功返回消息体总长度(byte), 失败返回-1.
using PackageHandler = std::function<
    int (ProtocolParameter const& para, std::vector<uint8_t>* out)>;

// 封装器定义, map<key, value>, key: 消息ID, value: 封装处理函数.
using Packager = std::map<uint16_t, PackageHandler>;

// 封装器初始化命令, 里面提供了一部分命令的封装功能.
int JT808FramePackagerInit(Packager* packager);

// 额外增加封装器支持命令.
bool JT808FramePackagerAppend(
    Packager* packager, std::pair<uint16_t, PackageHandler> const& pair);

// 额外增加封装器支持命令.
bool JT808FramePackagerAppend(Packager* packager,
                              uint16_t const& msg_id,
                              PackageHandler const& handler);

// 封装命令.
int JT808FramePackage(Packager const& packager,
                      ProtocolParameter const& para,
                      std::vector<uint8_t>* out);

}  // namespace libjt808

#endif  // JT808_PACKAGER_H_
