cmake_minimum_required(VERSION 3.20)
project(preciscan_pc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(acquire_cli
    tools/acquire_cli/main.cpp
    core/seq_metrics.cpp
    core/metrics_writer.cpp
    net/udp_receiver.cpp
    core/config.cpp
    net/udp_recorder.cpp
    io/replay_reader.cpp




)
target_include_directories(acquire_cli PRIVATE core net io)

add_executable(udp_sender_sim
    tools/udp_sender_sim/main.cpp
)

# Windows sockets (Winsock)
if (WIN32)
  target_link_libraries(acquire_cli PRIVATE ws2_32)
  target_link_libraries(udp_sender_sim PRIVATE ws2_32)
endif()
