#include <QApplication>

#include "../hardware/laser_manager.h"
#include "../net/mcu_listener.h"
#include "../net/ring_buffer_sink.h"
#include "../net/spsc_ring_buffer.h"
#include "ui/MainWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Preciscan");
    app.setApplicationVersion("2.1");

    // --- Backend nesneleri (UI ile paylasilan) ---
    SPSCRingBuffer ring(4096);
    RingBufferSink sink(ring);
    McuListener mcu(8080); // Varsayilan port, CLI'dan ayri
    LaserManager laser("LLT.dll", &sink);

    // --- Ana pencere ---
    MainWindow window(&mcu, &laser, &ring);
    window.show();

    return app.exec();
}
