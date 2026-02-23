cmake_minimum_required(VERSION 3.16)
project(PreciscanCore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- CORE kutuphanesi ---
add_library(preciscan_core STATIC
    core/config.cpp
    core/seq_metrics.cpp
    core/meta_writer.cpp
    core/metrics_writer.cpp
)
target_include_directories(preciscan_core PUBLIC core/)
target_compile_options(preciscan_core PRIVATE /W4)

# --- NET kutuphanesi ---
add_library(preciscan_net STATIC
    net/udp_receiver.cpp
    net/udp_recorder.cpp
    net/mcu_listener.cpp
)
target_include_directories(preciscan_net PUBLIC net/)
target_link_libraries(preciscan_net PUBLIC preciscan_core ws2_32)
target_compile_options(preciscan_net PRIVATE /W4)

# --- HARDWARE kutuphanesi ---
add_library(preciscan_hw STATIC
    hardware/laser_manager.cpp
    third_party/microepsilon/InterfaceLLT_2.cpp
    third_party/microepsilon/DllLoader.cpp
)
target_include_directories(preciscan_hw PUBLIC hardware/ third_party/microepsilon/)
target_include_directories(preciscan_hw PRIVATE hardware/)
target_link_libraries(preciscan_hw PUBLIC preciscan_core)
target_compile_options(preciscan_hw PRIVATE /W4)

# --- CLI aracı (donanim olmadan test icin) ---
add_executable(acquire_cli
    tools/acquire_cli/main.cpp
    io/replay_reader.cpp
    io/disk_writer.cpp
)
target_include_directories(acquire_cli PRIVATE io/)
target_link_libraries(acquire_cli PRIVATE preciscan_net preciscan_hw)
target_compile_options(acquire_cli PRIVATE /W4)

# --- UDP Simulator ---
add_executable(udp_sender_sim tools/udp_sender_sim/main.cpp)
target_link_libraries(udp_sender_sim PRIVATE ws2_32)
target_compile_options(udp_sender_sim PRIVATE /W4)

# --- Qt UI (Qt6 kurulu olmali: C:/Qt/6.x.x/msvc2022_64) ---
find_package(Qt6 QUIET COMPONENTS Widgets OpenGLWidgets)
if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    find_package(OpenGL REQUIRED)

    add_executable(preciscan_ui
        app/main_ui.cpp
        app/ui/MainWindow.cpp
        app/ui/panels/ScanPanel.cpp
        app/ui/panels/SetupPanel.cpp
        app/ui/panels/DiagnosticsPanel.cpp
        app/ui/panels/LayerItemWidget.cpp
        app/ui/state/AppStateMachine.cpp
        app/ui/widgets/SpeedControl.cpp
        app/ui/widgets/ExposureControl.cpp
        app/ui/widgets/InfoTipButton.cpp
        app/ui/widgets/VisualizerWidget.cpp
        app/ui/widgets/ProfileWidget.cpp
        app/controller/ScanController.cpp
        io/stl_loader.cpp
        io/ply_writer.cpp
        sim/mesh_slicer.cpp
        sim/laser_sim_worker.cpp
    )
    target_include_directories(preciscan_ui PRIVATE app/ app/ui/ io/ sim/)
    target_link_libraries(preciscan_ui PRIVATE
        Qt6::Widgets Qt6::OpenGLWidgets
        OpenGL::GL
        preciscan_net preciscan_hw
    )
    target_compile_options(preciscan_ui PRIVATE /W4 /utf-8)
    message(STATUS "Qt6 bulundu — preciscan_ui hedefi aktif.")
else()
    message(WARNING "Qt6 bulunamadi. preciscan_ui hedefi devre disi. Qt kurulumunu kontrol et.")
endif()

# --- Testler ---
enable_testing()
add_subdirectory(tests)
