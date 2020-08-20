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

// @File    :  parser.h
// @Version :  1.0
// @Time    :  2020/07/08 17:09:41
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_PARSER_H_
#define JT808_PARSER_H_

#include <stdint.h>

#include <functional>
#include <map>
#include <utility>
#include <vector>

#include "protocol_parameter.h"


namespace libjt808 {

// 消息体解析函数定义.
// 成功返回0, 失败返回-1.
using ParseHandler = std::function<
    int (std::vector<uint8_t> const& in, ProtocolParameter* para)>;

// 解析器定义, map<key, value>, key: 消息ID, value: 解析消息体处理函数.
using Parser = std::map<uint16_t, ParseHandler>;

// 解析器初始化命令, 里面提供了一部分命令的解析功能.
int JT808FrameParserInit(Parser* parser);

// 额外增加解析器支持命令.
bool JT808FrameParserAppend(
    Parser* parser, std::pair<uint16_t, ParseHandler> const& pair);
bool JT808FrameParserAppend(Parser* parser,
                            uint16_t const& msg_id,
                            ParseHandler const& handler);

// 重写解析器支持命令.
bool JT808FrameParserOverride(
    Parser* parser, std::pair<uint16_t, ParseHandler> const& pair);
bool JT808FrameParserOverride(Parser* parser,
                              uint16_t const& msg_id,
                              ParseHandler const& handler);

// 解析命令.
int JT808FrameParse(Parser const& parser,
                    std::vector<uint8_t> const& in,
                    ProtocolParameter* para);

}  // namespace libjt808

#endif  // JT808_PARSER_H_
