// MIT License
//
// Copyright (c) 2021 Yuming Meng
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

// @File    :  multimedia_upload.h
// @Version :  1.0
// @Time    :  2021/05/11 09:05:26
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_MULTIMEDIA_UPLOAD_H_
#define JT808_MULTIMEDIA_UPLOAD_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>


namespace libjt808 {

// 多媒体数据上传.
struct MultiMediaDataUpload {
  uint32_t media_id;  // 多媒体 ID. 必须 > 0.
  uint8_t media_type;  // 多媒体类型. 0: 图像；1: 音频；2: 视频.
  uint8_t media_format;  // 多媒体格式编码.
                         // 0: JPEG; 1: TIF; 2: MP3; 3: WAV; 4: WMV; 其他保留.
  uint8_t media_event;  // 事件项编码. 0: 平台下发指令; 1: 定时动作; 2: 抢劫报警触发;
                        // 3: 碰撞侧翻报警触发; 其他保留.
  uint8_t channel_id;  // 通道 ID.
  std::vector<uint8_t> loaction_report_body;  // 位置信息汇报(0x0200)消息体,
                                              // 表示多媒体数据的位置基本信息数据.
  std::vector<uint8_t> media_data;  // 多媒体数据包.
};

// 多媒体数据上传应答.
struct MultiMediaDataUploadResponse {
  uint16_t media_id;  // 多媒体ID.
  std::vector<uint16_t> reload_packet_ids;  // 需要重传的包序号ID.
};

}  // namespace libjt808

#endif  // JT808_MULTIMEDIA_UPLOAD_H_
