cmake_minimum_required(VERSION 3.16)
project(PreciscanCore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- CORE kutuphanesi (OS bagimsiz, saf is mantigi) ---
add_library(preciscan_core STATIC
    core/config.cpp
    core/seq_metrics.cpp
    core/meta_writer.cpp
    core/metrics_writer.cpp
)
target_include_directories(preciscan_core PUBLIC core/)
target_compile_options(preciscan_core PRIVATE /W4)

# --- NET kutuphanesi (WinSock gerektiren ag katmani) ---
add_library(preciscan_net STATIC
    net/udp_receiver.cpp
    net/udp_recorder.cpp
    net/mcu_listener.cpp
)
target_include_directories(preciscan_net PUBLIC net/)
target_link_libraries(preciscan_net PUBLIC preciscan_core ws2_32)
target_compile_options(preciscan_net PRIVATE /W4)

# --- HARDWARE kutuphanesi (Lazer SDK ve surucu katmani) ---
add_library(preciscan_hw STATIC
    hardware/laser_manager.cpp
    third_party/microepsilon/InterfaceLLT_2.cpp
    third_party/microepsilon/DllLoader.cpp
)
target_include_directories(preciscan_hw PUBLIC hardware/ third_party/microepsilon/)
target_link_libraries(preciscan_hw PUBLIC preciscan_core)
target_compile_options(preciscan_hw PRIVATE /W4)

# --- Ana uygulama (sadece giris noktasi, is mantigi kutuphanelerde) ---
add_executable(acquire_cli
    tools/acquire_cli/main.cpp
    io/replay_reader.cpp
)
target_link_libraries(acquire_cli PRIVATE preciscan_net preciscan_hw)
target_compile_options(acquire_cli PRIVATE /W4)

# --- UDP Simulator (bagimsiz, sadece ws2_32) ---
add_executable(udp_sender_sim tools/udp_sender_sim/main.cpp)
target_link_libraries(udp_sender_sim PRIVATE ws2_32)
target_compile_options(udp_sender_sim PRIVATE /W4)
