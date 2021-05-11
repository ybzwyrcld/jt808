add_executable(jt808_connect_emulator
  jt808_connect_emulator.cc
)
add_dependencies(jt808_connect_emulator jt808)
target_link_libraries(jt808_connect_emulator
  jt808
  pthread
)

add_executable(jt808_packager
  jt808_packager.cc
)
add_dependencies(jt808_packager jt808)
target_link_libraries(jt808_packager
  jt808
  pthread
)

add_executable(jt808_parser
  jt808_parser.cc
)
add_dependencies(jt808_parser jt808)
target_link_libraries(jt808_parser
  jt808
  pthread
)

# jt808_in_out_polygon_area_report示例程序依赖nmeaparser工程.
# 若要使用此程序必须先下载nmeaparser源码到此工程源码根目录下, 并在根目录
# 下的CMakeLists.txt文件中添加'add_subdirectory(nmeaparser)'行来包
# 含nmeaparser目录, 然后重新生成并编译.
#
# add_executable(jt808_in_out_polygon_area_report
#   jt808_in_out_polygon_area_report.cc
# )
# add_dependencies(jt808_in_out_polygon_area_report jt808 nmeaparser)
# target_link_libraries(jt808_in_out_polygon_area_report
#   jt808
#   pthread
#   nmeaparser
# )

add_executable(jt808_upgrade_request
  jt808_upgrade_request.cc
)
add_dependencies(jt808_upgrade_request jt808)
target_link_libraries(jt808_upgrade_request
  jt808
  pthread
)

add_executable(jt808_terminal_parameter_config
  jt808_terminal_parameter_config.cc
)
add_dependencies(jt808_terminal_parameter_config jt808)
target_link_libraries(jt808_terminal_parameter_config
  jt808
  pthread
)

add_executable(jt808_client
  jt808_client.cc
)
add_dependencies(jt808_client jt808)
target_link_libraries(jt808_client
  jt808
  pthread
)

add_executable(jt808_server
  jt808_server.cc
)
add_dependencies(jt808_server jt808)
target_link_libraries(jt808_server
  jt808
  pthread
)

add_executable(jt808_1hz_location_report_client
  jt808_1hz_location_report_client.cc
)
add_dependencies(jt808_1hz_location_report_client jt808)
target_link_libraries(jt808_1hz_location_report_client
  jt808
  pthread
)

add_executable(jt808_multimedia_upload_client
  jt808_multimedia_upload_client.cc
)
add_dependencies(jt808_multimedia_upload_client jt808)
target_link_libraries(jt808_multimedia_upload_client
  jt808
  pthread
)

add_executable(jt808_multimedia_upload_server
  jt808_multimedia_upload_server.cc
)
add_dependencies(jt808_multimedia_upload_server jt808)
target_link_libraries(jt808_multimedia_upload_server
  jt808
  pthread
)