cmake_minimum_required(VERSION 3.15)
project("Preciscan Core" CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Başlık dosyalarının aranacağı yollar
include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/io
    ${CMAKE_SOURCE_DIR}/net
    ${CMAKE_SOURCE_DIR}/third_party/microepsilon
)

# 1. Ana Program (Acquire CLI)
add_executable(acquire_cli 
    tools/acquire_cli/main.cpp 
    core/seq_metrics.cpp
    core/config.cpp 
    third_party/microepsilon/DllLoader.cpp
    third_party/microepsilon/InterfaceLLT_2.cpp
)

# Multi-Threading (pthread) desteği
find_package(Threads REQUIRED)
target_link_libraries(acquire_cli PRIVATE Threads::Threads)

# Windows işletim sistemi için Ağ (Socket) Kütüphanesi (ntohl vb. için)
if(WIN32)
    target_link_libraries(acquire_cli PRIVATE Ws2_32)
endif()

# 2. UDP Simülatörü
add_executable(udp_sender_sim 
    tools/udp_sender_sim/main.cpp
)

if(WIN32)
    target_link_libraries(udp_sender_sim PRIVATE Ws2_32)
endif()