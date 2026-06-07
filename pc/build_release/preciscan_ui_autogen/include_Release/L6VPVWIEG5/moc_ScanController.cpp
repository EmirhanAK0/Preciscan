/****************************************************************************
** Meta object code from reading C++ file 'ScanController.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../app/controller/ScanController.hpp"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScanController.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14ScanControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto ScanController::qt_create_metaobjectdata<qt_meta_tag_ZN14ScanControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScanController",
        "mcuConnectionChanged",
        "",
        "connected",
        "laserConnectionChanged",
        "isSimModeChanged",
        "isSim",
        "scanStarted",
        "scanStopped",
        "simProgressUpdated",
        "percent",
        "requestClearVisualizer",
        "mcuPacketReceived",
        "seq",
        "y_mm",
        "simProfileReceived",
        "theta_deg",
        "QList<QPointF>",
        "profile",
        "profileFrameReceived",
        "ScanProfileFrame",
        "frame",
        "pointCloudReady",
        "QList<QVector3D>",
        "cloud",
        "meshLoaded",
        "triangles",
        "logMessage",
        "level",
        "msg",
        "triggerModeChanged",
        "ScanTriggerMode",
        "mode",
        "historySizeChanged",
        "size",
        "connectMcu",
        "disconnectMcu",
        "connectLaser",
        "connectLaserSim",
        "stlPath",
        "disconnectLaser",
        "setDOffset",
        "mm",
        "setLateralOffset",
        "setResolution",
        "deg",
        "setRps",
        "rps",
        "setLaserProfileRate",
        "hz",
        "setLaserShutterUs",
        "us",
        "setLaserAutoShutter",
        "enabled",
        "setLaserMeasuringField",
        "field",
        "setLaserPointsPerProfile",
        "points",
        "setTriggerMode",
        "setSerialPort",
        "portName",
        "onEncoderTrigger",
        "angleDeg",
        "connectMcuSerial",
        "disconnectMcuSerial",
        "startScan",
        "stopScan",
        "saveCurrentScan",
        "path",
        "sendSerialCommand",
        "cmd",
        "sendZMove",
        "sendZHome",
        "sendLinHome",
        "applyFilterCylindrical",
        "radiusMm",
        "minZ",
        "maxZ",
        "applyManualDeletion",
        "QList<int>",
        "indicesToRemove",
        "undoLastFilter",
        "resetCloud",
        "consumeHardwarePackets"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'mcuConnectionChanged'
        QtMocHelpers::SignalData<void(bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Signal 'laserConnectionChanged'
        QtMocHelpers::SignalData<void(bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Signal 'isSimModeChanged'
        QtMocHelpers::SignalData<void(bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 6 },
        }}),
        // Signal 'scanStarted'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'scanStopped'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'simProgressUpdated'
        QtMocHelpers::SignalData<void(int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Signal 'requestClearVisualizer'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mcuPacketReceived'
        QtMocHelpers::SignalData<void(quint32, float)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UInt, 13 }, { QMetaType::Float, 14 },
        }}),
        // Signal 'simProfileReceived'
        QtMocHelpers::SignalData<void(float, const QVector<QPointF> &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 16 }, { 0x80000000 | 17, 18 },
        }}),
        // Signal 'profileFrameReceived'
        QtMocHelpers::SignalData<void(const ScanProfileFrame &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 20, 21 },
        }}),
        // Signal 'pointCloudReady'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
        // Signal 'meshLoaded'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 23, 26 },
        }}),
        // Signal 'logMessage'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 28 }, { QMetaType::QString, 29 },
        }}),
        // Signal 'triggerModeChanged'
        QtMocHelpers::SignalData<void(ScanTriggerMode)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 31, 32 },
        }}),
        // Signal 'historySizeChanged'
        QtMocHelpers::SignalData<void(int)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'connectMcu'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'disconnectMcu'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectLaser'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectLaserSim'
        QtMocHelpers::SlotData<void(const QString &)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 39 },
        }}),
        // Slot 'disconnectLaser'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setDOffset'
        QtMocHelpers::SlotData<void(float)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 42 },
        }}),
        // Slot 'setLateralOffset'
        QtMocHelpers::SlotData<void(float)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 42 },
        }}),
        // Slot 'setResolution'
        QtMocHelpers::SlotData<void(float)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 45 },
        }}),
        // Slot 'setRps'
        QtMocHelpers::SlotData<void(float)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 47 },
        }}),
        // Slot 'setLaserProfileRate'
        QtMocHelpers::SlotData<void(int)>(48, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 49 },
        }}),
        // Slot 'setLaserShutterUs'
        QtMocHelpers::SlotData<void(int)>(50, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 51 },
        }}),
        // Slot 'setLaserAutoShutter'
        QtMocHelpers::SlotData<void(bool)>(52, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 53 },
        }}),
        // Slot 'setLaserMeasuringField'
        QtMocHelpers::SlotData<void(const QString &)>(54, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 55 },
        }}),
        // Slot 'setLaserPointsPerProfile'
        QtMocHelpers::SlotData<void(int)>(56, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 57 },
        }}),
        // Slot 'setTriggerMode'
        QtMocHelpers::SlotData<void(ScanTriggerMode)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 31, 32 },
        }}),
        // Slot 'setSerialPort'
        QtMocHelpers::SlotData<void(const QString &)>(59, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 60 },
        }}),
        // Slot 'onEncoderTrigger'
        QtMocHelpers::SlotData<void(float)>(61, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 62 },
        }}),
        // Slot 'connectMcuSerial'
        QtMocHelpers::SlotData<void()>(63, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'disconnectMcuSerial'
        QtMocHelpers::SlotData<void()>(64, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startScan'
        QtMocHelpers::SlotData<void()>(65, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stopScan'
        QtMocHelpers::SlotData<void()>(66, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'saveCurrentScan'
        QtMocHelpers::SlotData<void(const QString &)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 68 },
        }}),
        // Slot 'sendSerialCommand'
        QtMocHelpers::SlotData<bool(const QString &)>(69, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 70 },
        }}),
        // Slot 'sendZMove'
        QtMocHelpers::SlotData<void(float)>(71, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 42 },
        }}),
        // Slot 'sendZHome'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'sendLinHome'
        QtMocHelpers::SlotData<void()>(73, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'applyFilterCylindrical'
        QtMocHelpers::SlotData<void(float, float, float)>(74, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 75 }, { QMetaType::Float, 76 }, { QMetaType::Float, 77 },
        }}),
        // Slot 'applyFilterCylindrical'
        QtMocHelpers::SlotData<void(float, float)>(74, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Float, 75 }, { QMetaType::Float, 76 },
        }}),
        // Slot 'applyFilterCylindrical'
        QtMocHelpers::SlotData<void(float)>(74, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Float, 75 },
        }}),
        // Slot 'applyManualDeletion'
        QtMocHelpers::SlotData<void(const QVector<int> &)>(78, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'undoLastFilter'
        QtMocHelpers::SlotData<void()>(81, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'resetCloud'
        QtMocHelpers::SlotData<void()>(82, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'consumeHardwarePackets'
        QtMocHelpers::SlotData<void()>(83, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScanController, qt_meta_tag_ZN14ScanControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScanController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ScanControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ScanControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14ScanControllerE_t>.metaTypes,
    nullptr
} };

void ScanController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScanController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->mcuConnectionChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->laserConnectionChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->isSimModeChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->scanStarted(); break;
        case 4: _t->scanStopped(); break;
        case 5: _t->simProgressUpdated((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->requestClearVisualizer(); break;
        case 7: _t->mcuPacketReceived((*reinterpret_cast<std::add_pointer_t<quint32>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 8: _t->simProfileReceived((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<QPointF>>>(_a[2]))); break;
        case 9: _t->profileFrameReceived((*reinterpret_cast<std::add_pointer_t<ScanProfileFrame>>(_a[1]))); break;
        case 10: _t->pointCloudReady((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 11: _t->meshLoaded((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 12: _t->logMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 13: _t->triggerModeChanged((*reinterpret_cast<std::add_pointer_t<ScanTriggerMode>>(_a[1]))); break;
        case 14: _t->historySizeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->connectMcu(); break;
        case 16: _t->disconnectMcu(); break;
        case 17: _t->connectLaser(); break;
        case 18: _t->connectLaserSim((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->disconnectLaser(); break;
        case 20: _t->setDOffset((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 21: _t->setLateralOffset((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 22: _t->setResolution((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 23: _t->setRps((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 24: _t->setLaserProfileRate((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->setLaserShutterUs((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->setLaserAutoShutter((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 27: _t->setLaserMeasuringField((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 28: _t->setLaserPointsPerProfile((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->setTriggerMode((*reinterpret_cast<std::add_pointer_t<ScanTriggerMode>>(_a[1]))); break;
        case 30: _t->setSerialPort((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 31: _t->onEncoderTrigger((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 32: _t->connectMcuSerial(); break;
        case 33: _t->disconnectMcuSerial(); break;
        case 34: _t->startScan(); break;
        case 35: _t->stopScan(); break;
        case 36: _t->saveCurrentScan((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 37: { bool _r = _t->sendSerialCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 38: _t->sendZMove((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 39: _t->sendZHome(); break;
        case 40: _t->sendLinHome(); break;
        case 41: _t->applyFilterCylindrical((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[3]))); break;
        case 42: _t->applyFilterCylindrical((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 43: _t->applyFilterCylindrical((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 44: _t->applyManualDeletion((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 45: _t->undoLastFilter(); break;
        case 46: _t->resetCloud(); break;
        case 47: _t->consumeHardwarePackets(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 44:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(bool )>(_a, &ScanController::mcuConnectionChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(bool )>(_a, &ScanController::laserConnectionChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(bool )>(_a, &ScanController::isSimModeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::scanStarted, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::scanStopped, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(int )>(_a, &ScanController::simProgressUpdated, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::requestClearVisualizer, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(quint32 , float )>(_a, &ScanController::mcuPacketReceived, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(float , const QVector<QPointF> & )>(_a, &ScanController::simProfileReceived, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const ScanProfileFrame & )>(_a, &ScanController::profileFrameReceived, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QVector<QVector3D> & )>(_a, &ScanController::pointCloudReady, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QVector<QVector3D> & )>(_a, &ScanController::meshLoaded, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QString & , const QString & )>(_a, &ScanController::logMessage, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(ScanTriggerMode )>(_a, &ScanController::triggerModeChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(int )>(_a, &ScanController::historySizeChanged, 14))
            return;
    }
}

const QMetaObject *ScanController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScanController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ScanControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ScanController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 48)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 48;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 48)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 48;
    }
    return _id;
}

// SIGNAL 0
void ScanController::mcuConnectionChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ScanController::laserConnectionChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ScanController::isSimModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ScanController::scanStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ScanController::scanStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ScanController::simProgressUpdated(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ScanController::requestClearVisualizer()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ScanController::mcuPacketReceived(quint32 _t1, float _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}

// SIGNAL 8
void ScanController::simProfileReceived(float _t1, const QVector<QPointF> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void ScanController::profileFrameReceived(const ScanProfileFrame & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void ScanController::pointCloudReady(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void ScanController::meshLoaded(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void ScanController::logMessage(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1, _t2);
}

// SIGNAL 13
void ScanController::triggerModeChanged(ScanTriggerMode _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void ScanController::historySizeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}
QT_WARNING_POP
