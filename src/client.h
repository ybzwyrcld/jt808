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
  // 获取当前服务线程运行状态.
  bool service_is_running(void) const {
    return service_is_running_;
  }

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
    parameter_.location_info.speed = static_cast<uint16_t>(speed/10);
    parameter_.location_info.bearing = static_cast<uint16_t>(bearing);
    parameter_.location_info.time.assign(timestamp.begin(), timestamp.end());
  }
  // 更新GNSS定位卫星数.
  // Args:
  //     num:  定位使用卫星数.
  // Returns:
  //     None.
  void UpdateGNSSSatelliteNumber(uint8_t const& num) {
    auto const& it = parameter_.location_extension.find(kGnssSatellites);
    if (it != parameter_.location_extension.end()) {
      it->second.clear();
      it->second.push_back(num);
    } else {
      parameter_.location_extension.insert(
          std::make_pair(kGnssSatellites, std::vector<uint8_t>{num}));
    }
  }
  // 更新GNSS定位解状态.
  // 此项为自定义终端参数, 非相关设备请忽略.
  // Args:
  //     fix:  定位状态: 1-单点定位. 2-伪距差分,
  //         4-RTK固定, 5-RTK浮点, 6-惯导.
  // Returns:
  //     None.
  void UpdateGNSSPositioningSolutionStatus(uint8_t const& fix) {
    auto const& it = parameter_.location_extension.find(kPositioningStatus);
    if (it != parameter_.location_extension.end()) {
      it->second.clear();
      it->second.push_back(fix);
    } else {
      parameter_.location_extension.insert(
          std::make_pair(kPositioningStatus, std::vector<uint8_t>{fix}));
    }
  }
  // 设置位置上报时间间隔.
  void set_location_report_inteval(uint8_t const& intv) {
    location_report_inteval_ = intv;
  }
  // 立即进行位置上报标志.
  enum ReportImmediatelyFlag {
    kAlarmOccurred = 0x1,
    kStateChanged = 0x2,
  };

  //
  // 终端参数相关.
  //
  // 获取终端心跳时间间隔.
  int GetTerminalHeartbeatInterval(uint32_t* interval) const {
    return PackagingTerminalParameterTerminalHeartBeatInterval(
        parameter_.terminal_parameters, interval);
  }
  // 以下为自定义终端参数, 非相关设备请忽略.
  // 获取GNSS模块输出语句配置.
  int GetGNSSModelLog(uint8_t* loggga_en, uint8_t* logrmc_en,
                      uint8_t* logatt_en, uint8_t* startup) const {
    return ParseTerminalParameterGNSSLog(
        parameter_.terminal_parameters, loggga_en, logrmc_en, logatt_en, startup);
  }
  // 设置GNSS模块输出语句.
  int SetGNSSModelLog(uint8_t const& loggga_en, uint8_t const& logrmc_en,
                      uint8_t const& logatt_en, uint8_t const& startup) {
    return PackagingTerminalParameterGNSSLog(
        loggga_en, logrmc_en, logatt_en, startup,
        &parameter_.terminal_parameters);
  }
  // 获取CDRadio模块配置.
  int GetCDRadio(uint32_t* bdrt, uint16_t* freq,
                 uint8_t* recv_mode, uint8_t* form_code, uint8_t* startup) const {
    return ParseTerminalParameterCDRadio(
        parameter_.terminal_parameters, bdrt, freq, recv_mode, form_code, startup);
  }
  // 设置CDRadio模块.
  int SetCDRadio(uint32_t const& bdrt, uint16_t const& freq,
                 uint8_t const& recv_mode, uint8_t const& form_code,
                 uint8_t const& startup) {
    return PackagingTerminalParameterCDRadio(
        bdrt, freq, recv_mode, form_code, startup,
        &parameter_.terminal_parameters);
  }
  // 获取Ntrip网络差分配置.
  int GetNtripCors(std::string* ip, uint16_t* port,
      std::string* user, std::string* pwd,
      std::string* mp, uint8_t* intv, uint8_t* startup) const {
    return ParseTerminalParameterNtripCors(parameter_.terminal_parameters,
        ip, port, user, pwd, mp, intv, startup);
  }
  // 设置Ntrip网络差分.
  int SetNtripCors(std::string const& ip, uint16_t const& port,
      std::string const& user, std::string const& pwd,
      std::string const& mp, uint8_t const& intv, uint8_t const& startup) {
    return PackagingTerminalParameterNtripCors(
        ip, port, user, pwd, mp, intv, startup,
        &parameter_.terminal_parameters);
  }
  // 获取Ntrip后台配置.
  int GetNtripService(std::string* ip, uint16_t* port,
      std::string* user, std::string* pwd,
      std::string* mp, uint8_t* intv, uint8_t* startup) const {
    return ParseTerminalParameterNtripService(parameter_.terminal_parameters,
        ip, port, user, pwd, mp, intv, startup);
  }
  // 设置Ntrip后台.
  int SetNtripService(std::string const& ip, uint16_t const& port,
      std::string const& user, std::string const& pwd,
      std::string const& mp, uint8_t const& intv, uint8_t const& startup) {
    return PackagingTerminalParameterNtripService(
        ip, port, user, pwd, mp, intv, startup,
        &parameter_.terminal_parameters);
  }
  // 获取JT808后台配置.
  int GetJT808Service(std::string* ip, uint16_t* port,
      std::string* phone,  uint8_t* intv, uint8_t* startup) const {
    return ParseTerminalParameterJT808Service(parameter_.terminal_parameters,
        ip, port, phone, intv, startup);
  }
  // 设置JT808后台.
  int SetJT808Service(std::string const& ip, uint16_t const& port,
      std::string const& phone, uint8_t const& intv, uint8_t const& startup) {
    return PackagingTerminalParameterJT808Service(
        ip, port, phone, intv, startup,
        &parameter_.terminal_parameters);
  }
  // 终端参数回调函数.
  using TerminalParameterCallback = std::function<void (/* 参数待定. */)>;
  // 设置终端参数回调函数.
  void OnSetTerminalParamete(TerminalParameterCallback const& callback) {
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
  void OnUpgrade(UpgradeCallback const& callback) {
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
  // Args:
  //     ids:  区域ID集.
  // Returns:
  //     None.
  void DeletePolygonAreaByIDs(std::vector<uint32_t> const& ids) {
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
  // 与服务端通信线程.
  void ThreadHandler(void);

  decltype(socket(0, 0, 0)) client_;  // 通用TCP连接socket.
  std::atomic_bool is_connected_;  // 与服务端TCP连接状态.
  std::atomic_bool is_authenticated_;  // 鉴权状态.
  std::string ip_;  // 服务端IP地址.
  int port_;  // 服务端端口.
  uint8_t location_report_inteval_;  // 位置信息上报时间间隔.
  uint16_t location_report_immediately_flag_;  // 立即进行位置上报标志.
  std::thread service_thread_;  // 服务线程.
  std::atomic_bool service_is_running_;  // 服务线程运行标志.
  TerminalParameterCallback terminal_parameter_callback_;  // 设置终端参数回调函数.
  UpgradeCallback upgrade_callback_;  // 下发终端升级包回调函数.
  Packager packager_;  // 通用JT808协议封装器.
  Parser parser_;  // 通用JT808协议解析器.
  ProtocolParameter parameter_;  // JT808协议参数.
  PolygonAreaSet polygon_areas_;  // 多边形区域信息集.
};

}  // namespace libjt808

#endif  // JT808_CLIENT_H_
