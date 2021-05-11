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

// @File    :  client.h
// @Version :  1.0
// @Time    :  2020/07/17 10:05:36
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_CLIENT_H_
#define JT808_CLIENT_H_

#if defined(__linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <list>
#include <mutex>

#include "packager.h"
#include "parser.h"
#include "protocol_parameter.h"
#include "terminal_parameter.h"


namespace libjt808 {

// JT808终端.
// 已实现了终端注册, 终端鉴权, 心跳包, 位置信息汇报功能.
//
// Example:
//     JT808Client client;
//     client.Init();
//     client.SetRemoteAccessPoint("127.0.0.1", 8888);
//     if ((client.ConnectRemote() == 0) &&
//         (client.JT808ConnectionAuthentication() == 0)) {
//       client.Run();
//       std::this_thread::sleep_for(std::chrono::seconds(1));
//       while (client.service_is_running()) {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//       }
//       client.Stop();
//     }
//
//     // 在其它位置调用client.UpdateLocation(args)来更新位置基本信息.
//     // 在其它位置调用client.SetAlarmBit(args)来更新报警位.
//     // 在其它位置调用client.SetStatusBit(args)来更新状态位.
class JT808Client {
 public:
  JT808Client();
  ~JT808Client();
  void Init(void);
  //
  // 连接服务端相关.
  //
  void SetRemoteAccessPoint(std::string const&ip, int const& port) {
    ip_ = ip;
    port_ = port;
  }
  // 设置终端手机号.
  void SetTerminalPhoneNumber(std::string const& phone) {
    parameter_.msg_head.phone_num.clear();
    parameter_.msg_head.phone_num.assign(phone.begin(), phone.end());
  }
  // 连接到远程服务器.
  int ConnectRemote(void);
  // JT808连接认证.
  int JT808ConnectionAuthentication(void);

  //
  // 终端注册相关.
  //
  // 设置终端注册信息.
  void SetTerminalRegisterInfo(RegisterInfo const& info) {
    parameter_.register_info = info;
  }
  // 设置终端注册信息.
  // Args:
  //     p_id:  省域ID.
  //     c_id:  市县域ID.
  //     m_id:  制造商ID, 最长5字节.
  //     t_model:  终端型号, 最长20字节.
  //     t_id:  终端ID, 最长7字节.
  //     c_color:  车牌颜色.
  //     c_num:  车牌号码.
  // Returns:
  //     None.
  void SetTerminalRegisterInfo(uint16_t const& p_id, uint16_t const& c_id,
      std::vector<uint8_t> const& m_id, std::vector<uint8_t> const& t_model,
      std::vector<uint8_t> const& t_id,
      uint8_t const& c_color, std::string const& c_num) {
    parameter_.register_info.province_id = p_id;
    parameter_.register_info.city_id = c_id;
    parameter_.register_info.manufacturer_id.clear();
    parameter_.register_info.manufacturer_id.assign(m_id.begin(), m_id.end());
    parameter_.register_info.terminal_model.clear();
    parameter_.register_info.terminal_model.assign(
        t_model.begin(), t_model.end());
    parameter_.register_info.terminal_id.clear();
    parameter_.register_info.terminal_id.assign(t_id.begin(), t_id.end());
    parameter_.register_info.car_plate_color = c_color;
    if (c_color != kVin) {
      parameter_.register_info.car_plate_num.clear();
      parameter_.register_info.car_plate_num.assign(
          c_num.begin(), c_num.end());
    }
  }

  //
  // 服务线程运行与终止.
  //
  // 启动服务线程.
  void Run(void);
  // 停止服务线程.
  void Stop(void);
  // 等待所有缓存消息发送完成或等待超时后再停止服务线程.
  void WattingStop(int const& timeout_msec);
  // 获取当前服务线程运行状态.
  bool service_is_running(void) const {
    return service_is_running_;
  }

  //
  //  外部获取和设置当前的通用消息体解析和封装函数, 用于重写或新增命令支持.
  //  必须在调用Init()成员函数后才可以使用.
  //
  // 获取通用JT808协议封装器.
  Packager& packager(void) { return packager_; }
  Packager const& packager(void) const { return packager_; }
  void packager(Packager* packager) const {
    if (packager == nullptr) return;
    *packager = packager_;
  }
  // 设置通用JT808协议封装器.
  void set_packager(Packager const& packager) { packager_ = packager; }
  // 获取通用JT808协议解析器.
  Parser& parser(void) { return parser_; }
  Parser const& parser(void) const {  return parser_; }
  void parser(Parser* parser) const {
    if (parser == nullptr) return;
    *parser = parser_;
  }
  // 设置通用JT808协议解析器.
  void set_parser(Parser const& parser) { parser_ = parser; }

  //
  // 位置上报相关.
  //
  // 设置报警标识位.
  void SetAlarmBit(uint32_t const& alarm) {
    parameter_.location_info.alarm.value = alarm;
  }
  // 获取报警标识位.
  uint32_t const& alarm_bit(void) const {
    return parameter_.location_info.alarm.value;
  }
  // 设置进出区域报警标识位.
  // Args:
  //     in:  进出区域标志, 0-进入, 1- 离开.
  // Returns:
  //     None.
  void SetInOutAreaAlarmBit(uint8_t const& in) {
    parameter_.location_info.alarm.bit.in_out_area = in;
    location_report_immediately_flag_ |= kAlarmOccurred;
  }
  // 设置进出区域报警位置信息附加项.
  // Args:
  //     item:  进出区域附加项值, 类型(BYTE)+区域ID(DWORD)+方向(BYTE).
  // Returns:
  //     None.
  void SetInOutAreaAlarmExtension(std::vector<uint8_t> const& item) {
    auto const& it = parameter_.location_extension.find(kAccessAreaAlarm);
    if (it != parameter_.location_extension.end()) {
      it->second.assign(item.begin(), item.end());
    } else {
      parameter_.location_extension.insert(
          std::make_pair(kAccessAreaAlarm, item));
    }
  }
  // 设置状态位.
  void SetStatusBit(uint32_t const& status) {
    parameter_.location_info.status.value = status;
    location_report_immediately_flag_ |= kStateChanged;
  }
  // 获取状态位.
  uint32_t const& status_bit(void) const {
    return parameter_.location_info.status.value;
  }
  // 更新位置基本信息.
  // Args:
  //     info: 位置基本信息.
  // Returns:
  //     None.
   void UpdateLocation(LocationBasicInformation const& info) {
     parameter_.location_info = info;
   }
  // 更新位置基本信息.
  // Args:
  //     latitude:  纬度值, 单位为度(°).
  //     longitude:  经度值, 单位为度(°).
  //     altitude:  高程, 单位为米(m).
  //     speed:  速度, 单位为千米每小时(km/h).
  //     bearing:  方向.
  //     timestamp:  GMT+8时间戳, YYMMDDhhmmss格式.
  // Returns:
  //     None.
  void UpdateLocation(double const& latitude, double const& longitude,
                      float const& altitude, float const& speed,
                      float const& bearing, std::string const& timestamp) {
    parameter_.location_info.latitude = static_cast<uint32_t>(latitude*1e6);
    parameter_.location_info.longitude = static_cast<uint32_t>(longitude*1e6);
    parameter_.location_info.altitude = static_cast<uint16_t>(altitude);
    parameter_.location_info.speed = static_cast<uint16_t>(speed*10);
    parameter_.location_info.bearing = static_cast<uint16_t>(bearing);
    parameter_.location_info.time.assign(timestamp.begin(), timestamp.end());
  }
  // 获取位置信息附加项.
  int GetLocationExtension(LocationExtensions* items) {
    if (items == nullptr) return -1;
    items->clear();
    items->insert(parameter_.location_extension.begin(),
                  parameter_.location_extension.end());
    return 0;
  }
  // 获取位置信息附加项.
  LocationExtensions const& GetLocationExtension(void) const {
    return parameter_.location_extension;
  }
  // 获取位置信息附加项.
  LocationExtensions& GetLocationExtension(void) {
    return parameter_.location_extension;
  }
  // 设置位置上报时间间隔.
  // 如果希望每次上报的数据更加准确, 需将msg_generate_outside设为true,
  // 在解析完定位模块数据后调用GenerateLocationReportMsgNow()方法,
  // 立刻生成当前时刻的位置上报信息.
  void set_location_report_inteval(uint8_t const& intv,
      bool const& msg_generate_outside = false) {
    location_report_inteval_ = intv;
    location_report_msg_generate_outside_.store(
        msg_generate_outside);
  }
  // 立即进行位置上报标志.
  enum ReportImmediatelyFlag {
    kAlarmOccurred = 0x1,  // 报警标志改变.
    kStateChanged = 0x2,  // 状态标志改变.
  };
  // 立刻生成一条位置上报消息.
  // 仅在外部控制位置上报时调用.
  void GenerateLocationReportMsgNow(void);

  //
  // 终端参数相关.
  //
  // 获取终端心跳时间间隔.
  int GetTerminalHeartbeatInterval(uint32_t* interval) const {
    return ParseTerminalParameterTerminalHeartBeatInterval(
        parameter_.terminal_parameters, interval);
  }
  // 设置终端心跳时间间隔.
  int SetTerminalHeartbeatInterval(uint32_t const& interval) {
    return PackagingTerminalParameterTerminalHeartBeatInterval(
        interval, &parameter_.terminal_parameters);
  }
  // 获取所有终端参数.
  int GetTerminalParameters(TerminalParameters* para) const {
    if (para == nullptr) return -1;
    para->clear();
    para->insert(parameter_.terminal_parameters.begin(),
                 parameter_.terminal_parameters.end());
    return 0;
  }
  // 获取所有终端参数.
  TerminalParameters const& GetTerminalParameters(void) const {
    return parameter_.terminal_parameters;
  }
  // 获取所有终端参数.
  TerminalParameters& GetTerminalParameters(void) {
    return parameter_.terminal_parameters;
  }
  // 设置所有终端参数.
  void SetTerminalParameters(TerminalParameters const& para){
    parameter_.terminal_parameters.clear();
    parameter_.terminal_parameters.insert(para.begin(), para.end());
  }
  // 终端参数回调函数.
  using TerminalParameterCallback = std::function<void (void/* 参数待定. */)>;
  // 设置平台配置修改终端参数时的回调函数.
  void OnTerminalParameteUpdated(TerminalParameterCallback const& callback) {
    terminal_parameter_callback_ = callback;
  }

  //
  // 升级相关.
  //
  void UpgradeResultReport(uint8_t const& result);
  // 下发升级包回调函数.
  // Args:
  //     type:  升级类型.
  //     data:  升级包缓存地址.
  //     size:  升级包大小.
  // Returns:
  //     None.
  using UpgradeCallback = std::function<
      void (uint8_t const& type, char const* data, int const& size)>;
  // 设置下发升级包回调函数.
  void OnUpgraded(UpgradeCallback const& callback) {
    upgrade_callback_ = callback;
  }

  //
  //  区域路线相关.
  //
  // 获取当前多边形区域信息集.
  PolygonAreaSet const& polygon_areas(void) const {
    return polygon_areas_;
  }
  // 获取所有多边形区域.
  int GetAllPolygonArea(PolygonAreaSet* areas) const {
    if (areas == nullptr || polygon_areas_.empty()) return -1;
    areas->clear();
    areas->insert(polygon_areas_.begin(), polygon_areas_.end());
    return 0;
  }
  // 获取指定ID的多边形区域.
  int GetPolygonAreaByID(uint32_t const& id, PolygonArea* area) const {
    if (area == nullptr || polygon_areas_.empty()) return -1;
    auto const& it = polygon_areas_.find(id);
    if (it != polygon_areas_.end()) return -1;
    *area = it->second;
    return 0;
  }
  // 新增一个多边形区域.
  // Args:
  //     area:  多边形区域.
  // Returns:
  //     区域ID已存在返回-1, 否则返回0.
  int AddPolygonArea(PolygonArea const& area) {
    auto const& id = area.area_id;
    return (polygon_areas_.insert(std::make_pair(id, area)).second - 1);
  }
  // 新增一个多边形区域.
  // Args:
  //     id:  区域ID.
  //     attr:  区域属性.
  //     begin_time:  起始时间, 格式为"YYMMDDhhmmss";
  //     end_time:  结束时间, 格式为"YYMMDDhhmmss";
  //     max_speed:  最高时速, km/h;
  //     overspeed_time:  超速持续时间, s;
  //     vertices:  区域顶点, 按顺时针顺序依次存储;
  // Returns:
  //     区域ID已存在返回-1, 否则返回0.
  int AddPolygonArea(uint32_t const& id, uint16_t const& attr,
                     std::string const& begin_time,
                     std::string const& end_time,
                     uint16_t const& max_speed,
                     uint8_t const& overspeed_time,
                     std::vector<LocationPoint> const& vertices) {
    PolygonArea area = {id, AreaAttribute {attr}, begin_time, end_time,
                        max_speed, overspeed_time, vertices};
    return (polygon_areas_.insert(std::make_pair(id, area)).second - 1);
  }
  // 更新一个多边形区域信息.
  // 若区域ID不存在, 直接插入, 否则更新原有区域信息.
  // Args:
  //     id:  区域ID.
  //     attr:  区域属性.
  //     begin_time:  起始时间, 格式为"YYMMDDhhmmss";
  //     end_time:  结束时间, 格式为"YYMMDDhhmmss";
  //     max_speed:  最高时速, km/h;
  //     overspeed_time:  超速持续时间, s;
  //     vertices:  区域顶点, 按顺时针顺序依次存储;
  // Returns:
  //     None.
  void UpdatePolygonArea(uint32_t const& id, uint16_t const& attr,
                         std::string const& begin_time,
                         std::string const& end_time,
                         uint16_t const& max_speed,
                         uint8_t const& overspeed_time,
                         std::vector<LocationPoint> const& vertices) {
    PolygonArea area = {id, AreaAttribute {attr}, begin_time, end_time,
                        max_speed, overspeed_time, vertices};
    polygon_areas_[id] = area;
  }
  // 更新指定的多边形区域.
  // Args:
  //     area:  多边形区域.
  // Returns:
  //     None.
  void UpdatePolygonAreaByArea(PolygonArea const& area) {
    polygon_areas_[area.area_id] = area;
  }
  // 更新指定的多边形区域.
  // Args:
  //     areas:  多边形区域信息集.
  // Returns:
  //     None.
  void UpdatePolygonAreaByAreas(PolygonAreaSet const& areas) {
    for (auto const& item : areas) {
      polygon_areas_[item.first] = item.second;
    }
  }
  // 删除指定ID多边形区域.
  // Args:
  //     id:  区域ID.
  // Returns:
  //     None.
  void DeletePolygonAreaByID(uint32_t const& id) {
    auto const& it = polygon_areas_.find(id);
    if (it != polygon_areas_.end()) polygon_areas_.erase(it);
  }
  // 删除指定ID多边形区域.
  // 区域ID集为空时, 删除所有多边形区域信息.
  // Args:
  //     ids:  区域ID集.
  // Returns:
  //     None.
  void DeletePolygonAreaByIDs(std::vector<uint32_t> const& ids) {
     if (ids.empty()) {
      DeleteAllPolygonArea();
      return;
    }
    for (auto const& id : ids) {
      DeletePolygonAreaByID(id);
    }
  }
  // 删除所有多边形区域.
  // Args:
  //     None.
  // Returns:
  //     None.
  void DeleteAllPolygonArea(void) {
    polygon_areas_.clear();
  }
  // 多边形区域回调函数.
  using PolygonAreaCallback = std::function<void (void/* 参数待定. */)>;
  // 设置平台配置修改多边形区域信息时的回调函数.
  void OnPolygonAreaUpdated(PolygonAreaCallback const& callback) {
    polygon_area_callback_ = callback;
  }
  //
  // 多媒体数据上传.
  //
  // Args:
  //     path: 上传JPEG图片路径.
  //     location_basic: 位置基本信息封装.
  // Returns:
  //     None.
  int MultimediaUpload(char const* path,
      std::vector<uint8_t> const& location_basic);

  // 通用消息封装和发送函数.
  // Args:
  //     msg_id:  消息ID.
  // Returns:
  //     成功返回0, 失败返回-1.
  int PackagingAndSendMessage(uint32_t const& msg_id);

  // 通用消息接收和解析函数.
  // 阻塞函数.
  // 服务线程启用后禁止调用.
  // Args:
  //     timeout:  超时时间, 单位秒(s).
  // Returns:
  //     成功返回0, 失败返回-1.
  int ReceiveAndParseMessage(int const& timeout);

 private:
  // 生成一条消息.
  int PackagingMessage(uint32_t const& msg_id, std::vector<uint8_t>* out);
  // 生成一条消息, 存放在通用消息列表.
  int PackagingGeneralMessage(uint32_t const& msg_id);
  // 发送一条消息.
  int SendMessage(std::vector<uint8_t> const& msg);
  // 主线程处理函数.
  void ThreadHandler(void);
  // 发送消息到服务端线程处理函数.
  void SendHandler(std::atomic_bool *const running);
  // 接收服务端消息线程处理函数.
  void ReceiveHandler(std::atomic_bool *const running);

  std::atomic_bool manual_deal_;  // 手动处理标志.
  std::mutex msg_generate_mutex_;  // 消息生成互斥锁, 保证消息流水号唯一性.
  decltype(socket(0, 0, 0)) client_;  // 通用TCP连接socket.
  std::atomic_bool is_connected_;  // 与服务端TCP连接状态.
  std::atomic_bool is_authenticated_;  // 鉴权状态.
  std::string ip_;  // 服务端IP地址.
  int port_;  // 服务端端口.
  uint8_t location_report_inteval_;  // 位置信息上报时间间隔.
  uint16_t location_report_immediately_flag_;  // 立即进行位置上报标志.
  std::atomic_bool location_report_msg_generate_outside_;  // 外部控制生成位置上报信息.
  std::thread service_thread_;  // 服务线程.
  std::atomic_bool service_is_running_;  // 服务线程运行标志.
  TerminalParameterCallback terminal_parameter_callback_;  // 修改终端参数回调函数.
  UpgradeCallback upgrade_callback_;  // 下发终端升级包回调函数.
  PolygonAreaCallback polygon_area_callback_;  // 修改多边形区域回调函数.
  Packager packager_;  // 通用JT808协议封装器.
  Parser parser_;  // 通用JT808协议解析器.
  std::list<std::vector<uint8_t>> location_report_msg_;  // 位置上报消息列表.
  std::list<std::vector<uint8_t>> general_msg_;  // 除位置上报消息外的消息列表.
  PolygonAreaSet polygon_areas_;  // 多边形区域信息集.
  ProtocolParameter parameter_;  // JT808协议参数.
};

}  // namespace libjt808

#endif  // JT808_CLIENT_H_
