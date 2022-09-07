cmake_minimum_required(VERSION 3.14)
project(tcpstack)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============= debug helpers ============

set(CMAKE_VERBOSE_MAKEFILE off)

# Collect all currently added targets in all subdirectories
#
# Parameters: 1) _result: the list containing all found targets 2) _dir: root
# directory to start looking from
#
# reference:
# https://stackoverflow.com/questions/60211516/programmatically-get-all-targets-in-a-cmake-project
function(get_all_targets _result _dir)
  get_property(
    _subdirs
    DIRECTORY "${_dir}"
    PROPERTY SUBDIRECTORIES)
  foreach(_subdir IN LISTS _subdirs)
    get_all_targets(${_result} "${_subdir}")
  endforeach()

  get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
  set(${_result}
      ${${_result}} ${_sub_targets}
      PARENT_SCOPE)
endfunction()

# =============== Pthread ===============

# add this line to the target: target_link_libraries(target Threads::Threads)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# =============== PCAP ===============

add_subdirectory(deps/libpcap)
include_directories(deps/libpcap)

# =============== glog ==================

# we should include glog before gtest
add_subdirectory(deps/glog)

# =============== gtest ===============

add_subdirectory(deps/googletest)
set(GOOGLETEST_DRIVER ${CMAKE_CURRENT_BINARY_DIR}/google_driver.cc)
file(
  WRITE ${GOOGLETEST_DRIVER}
  "#include\"gtest/gtest.h\"\n\n" "int main(int argc, char* argv[]) {\n"
  "\t::testing::InitGoogleTest(&argc, argv);\n" "\treturn RUN_ALL_TESTS();\n"
  "}\n")
set(gtest_LIBRARY gtest gtest_main)

# =============== backward-cpp =============

add_subdirectory(deps/backward-cpp)

# =============== tcpstack ===============

# aux_source_directory(src SOURCE_FILES)
set(SOURCE_FILES
    src/base/alarm.cpp
    src/base/checksum.cpp
    src/base/data_reader.cpp
    src/base/data_writer.cpp
    src/base/epoll_alarm.cpp
    src/base/epoll_alarm_factory.cpp
    src/base/epoll_server.cpp
    src/base/rand_generator.cpp
    src/base/ring_buffer.cpp
    src/base/time_base.cpp
    src/ether/device.cpp
    src/ether/device_manager.cpp
    src/ether/mac_address.cpp
    src/ip/ip_address.cpp
    src/ip/ip_layer.cpp
    src/ip/routing_table.cpp
    src/posix/listen_dispatcher.cpp
    src/posix/posix_socket.cpp
    src/posix/protocol_stack.cpp
    src/posix/reset_dispatcher.cpp
    src/posix/segment_dispatcher.cpp
    src/posix/socket_struct.cpp
    src/tcp/segment.cpp
    src/tcp/segment_factory.cpp
    src/tcp/send_buffer.cpp
    src/tcp/socket_address.cpp
    src/tcp/socket_session.cpp)

add_library(tcpstack SHARED ${SOURCE_FILES})
# target_compile_options(tcpstack PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(tcpstack glog)

set(BUNDLE tcpstack pcap Threads::Threads pcap_static)

# add_executable(tcpstacktest main.cpp eval/wrap_null.c)
# target_link_libraries(tcpstacktest PUBLIC ${BUNDLE})

# file(GLOB_RECURSE TEST_FILES *_test.cpp)
set(TEST_FILES
    src/ip/ip_address_test.cpp
    src/ip/routing_table_test.cpp
    src/base/ring_buffer_test.cpp
    eval/wrap_null.c
    src/util/mock_alarm_factory.cpp
    src/util/mock_ip_layer.cpp
    src/util/mock_alarm.cpp
    src/tcp/socket_session_test.cpp)

# Enable gtest

include(GoogleTest)
enable_testing()
add_executable(runUnitTests ${GOOGLETEST_DRIVER} ${TEST_FILES}
                            ${BACKWARD_ENABLE})
target_link_libraries(runUnitTests ${BUNDLE} ${gtest_LIBRARY})
add_backward(runUnitTests)
gtest_discover_tests(runUnitTests)

# set up network namespace

add_library(setns OBJECT eval/setns.c eval/clean_up.c)

# =============== evaluation ===============

set(EVAL_FILES eval/unp.c)

add_library(unp OBJECT ${EVAL_FILES})

set(WARP_FUNC
    socket
    bind
    listen
    accept
    connect
    read
    write
    getaddrinfo
    close)
list(TRANSFORM WARP_FUNC PREPEND "-Wl,--wrap,")
string(REPLACE ";" " " WARP_FUNC_STR "${WARP_FUNC}")
target_link_options(tcpstack PRIVATE ${WARP_FUNC})

function(add_wrapped_executable TARGETNAME)
  add_executable(${TARGETNAME} "eval/${TARGETNAME}.c" ${BACKWARD_ENABLE})
  add_backward(${TARGETNAME})
  target_link_options(${TARGETNAME} PRIVATE ${WARP_FUNC})
  target_link_libraries(${TARGETNAME} PRIVATE unp)
  target_link_libraries(${TARGETNAME} PRIVATE ${BUNDLE})
  target_link_libraries(${TARGETNAME} PRIVATE setns)
endfunction()

add_wrapped_executable(echo_client)
add_wrapped_executable(echo_server)
add_wrapped_executable(router)
add_wrapped_executable(rattle_client)

# =============== clang-format ===============

file(
  GLOB_RECURSE ALL_FILES_TO_FORMAT
  LIST_DIRECTORIES false
  src/*.c src/*.cpp src/*.h eval/*)

add_custom_target(clang-format COMMAND clang-format-14 -i
                                       ${ALL_FILES_TO_FORMAT})
