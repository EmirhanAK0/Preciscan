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
        "icpMergeFinished",
        "QList<QVector3D>",
        "mergedCloud",
        "processingStarted",
        "taskName",
        "processingFinished",
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
        "activeLayerChanged",
        "id",
        "layersUpdated",
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
        "direction",
        "stopScan",
        "saveCurrentScan",
        "path",
        "sendSerialCommand",
        "cmd",
        "sendZMove",
        "sendZHome",
        "sendLinHome",
        "mergeWithICPAsync",
        "target",
        "source",
        "isInverse",
        "mergeSelectedLayers",
        "QList<int>",
        "layerIds",
        "generateMeshAsync",
        "clearMesh",
        "setActiveLayer",
        "deleteLayer",
        "setLayerName",
        "name",
        "setLayerZOffset",
        "addNewLayer",
        "applyFilterCylindrical",
        "radiusMm",
        "minZ",
        "maxZ",
        "applyFilterStatistical",
        "meanK",
        "stdDevThresh",
        "applyFilterRadius",
        "minNeighbors",
        "applyFilterVoxelGrid",
        "leafSizeMm",
        "clearHistory",
        "applyManualDeletion",
        "indicesToRemove",
        "undoLastFilter",
        "resetCloud",
        "consumeHardwarePackets",
        "onMeshFinished",
        "onFilterFinished",
        "onIcpFinished"
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
        // Signal 'icpMergeFinished'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Signal 'processingStarted'
        QtMocHelpers::SignalData<void(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 },
        }}),
        // Signal 'processingFinished'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestClearVisualizer'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mcuPacketReceived'
        QtMocHelpers::SignalData<void(quint32, float)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UInt, 19 }, { QMetaType::Float, 20 },
        }}),
        // Signal 'simProfileReceived'
        QtMocHelpers::SignalData<void(float, const QVector<QPointF> &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 22 }, { 0x80000000 | 23, 24 },
        }}),
        // Signal 'profileFrameReceived'
        QtMocHelpers::SignalData<void(const ScanProfileFrame &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 26, 27 },
        }}),
        // Signal 'pointCloudReady'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 29 },
        }}),
        // Signal 'meshLoaded'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 31 },
        }}),
        // Signal 'logMessage'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 33 }, { QMetaType::QString, 34 },
        }}),
        // Signal 'triggerModeChanged'
        QtMocHelpers::SignalData<void(ScanTriggerMode)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 36, 37 },
        }}),
        // Signal 'historySizeChanged'
        QtMocHelpers::SignalData<void(int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 39 },
        }}),
        // Signal 'activeLayerChanged'
        QtMocHelpers::SignalData<void(int)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 41 },
        }}),
        // Signal 'layersUpdated'
        QtMocHelpers::SignalData<void()>(42, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectMcu'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'disconnectMcu'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectLaser'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectLaserSim'
        QtMocHelpers::SlotData<void(const QString &)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 47 },
        }}),
        // Slot 'disconnectLaser'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setDOffset'
        QtMocHelpers::SlotData<void(float)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 50 },
        }}),
        // Slot 'setLateralOffset'
        QtMocHelpers::SlotData<void(float)>(51, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 50 },
        }}),
        // Slot 'setResolution'
        QtMocHelpers::SlotData<void(float)>(52, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 53 },
        }}),
        // Slot 'setRps'
        QtMocHelpers::SlotData<void(float)>(54, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 55 },
        }}),
        // Slot 'setLaserProfileRate'
        QtMocHelpers::SlotData<void(int)>(56, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 57 },
        }}),
        // Slot 'setLaserShutterUs'
        QtMocHelpers::SlotData<void(int)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 59 },
        }}),
        // Slot 'setLaserAutoShutter'
        QtMocHelpers::SlotData<void(bool)>(60, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 61 },
        }}),
        // Slot 'setLaserMeasuringField'
        QtMocHelpers::SlotData<void(const QString &)>(62, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 63 },
        }}),
        // Slot 'setLaserPointsPerProfile'
        QtMocHelpers::SlotData<void(int)>(64, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 65 },
        }}),
        // Slot 'setTriggerMode'
        QtMocHelpers::SlotData<void(ScanTriggerMode)>(66, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 36, 37 },
        }}),
        // Slot 'setSerialPort'
        QtMocHelpers::SlotData<void(const QString &)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 68 },
        }}),
        // Slot 'onEncoderTrigger'
        QtMocHelpers::SlotData<void(float)>(69, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 70 },
        }}),
        // Slot 'connectMcuSerial'
        QtMocHelpers::SlotData<void()>(71, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'disconnectMcuSerial'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startScan'
        QtMocHelpers::SlotData<void(int)>(73, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 74 },
        }}),
        // Slot 'startScan'
        QtMocHelpers::SlotData<void()>(73, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'stopScan'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'saveCurrentScan'
        QtMocHelpers::SlotData<void(const QString &)>(76, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 77 },
        }}),
        // Slot 'sendSerialCommand'
        QtMocHelpers::SlotData<bool(const QString &)>(78, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 79 },
        }}),
        // Slot 'sendZMove'
        QtMocHelpers::SlotData<void(float)>(80, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 50 },
        }}),
        // Slot 'sendZHome'
        QtMocHelpers::SlotData<void()>(81, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'sendLinHome'
        QtMocHelpers::SlotData<void()>(82, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'mergeWithICPAsync'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &, const QVector<QVector3D> &, bool)>(83, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 84 }, { 0x80000000 | 12, 85 }, { QMetaType::Bool, 86 },
        }}),
        // Slot 'mergeSelectedLayers'
        QtMocHelpers::SlotData<void(const QVector<int> &, const QString &)>(87, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 88, 89 }, { QMetaType::QString, 37 },
        }}),
        // Slot 'generateMeshAsync'
        QtMocHelpers::SlotData<void()>(90, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'clearMesh'
        QtMocHelpers::SlotData<void()>(91, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setActiveLayer'
        QtMocHelpers::SlotData<void(int)>(92, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 41 },
        }}),
        // Slot 'deleteLayer'
        QtMocHelpers::SlotData<void(int)>(93, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 41 },
        }}),
        // Slot 'setLayerName'
        QtMocHelpers::SlotData<void(int, const QString &)>(94, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 41 }, { QMetaType::QString, 95 },
        }}),
        // Slot 'setLayerZOffset'
        QtMocHelpers::SlotData<void(int, float)>(96, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 41 }, { QMetaType::Float, 50 },
        }}),
        // Slot 'addNewLayer'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &, const QString &)>(97, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 65 }, { QMetaType::QString, 95 },
        }}),
        // Slot 'addNewLayer'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &)>(97, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 12, 65 },
        }}),
        // Slot 'applyFilterCylindrical'
        QtMocHelpers::SlotData<void(float, float, float)>(98, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 99 }, { QMetaType::Float, 100 }, { QMetaType::Float, 101 },
        }}),
        // Slot 'applyFilterStatistical'
        QtMocHelpers::SlotData<void(int, float)>(102, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 103 }, { QMetaType::Float, 104 },
        }}),
        // Slot 'applyFilterRadius'
        QtMocHelpers::SlotData<void(float, int)>(105, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 99 }, { QMetaType::Int, 106 },
        }}),
        // Slot 'applyFilterVoxelGrid'
        QtMocHelpers::SlotData<void(float)>(107, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 108 },
        }}),
        // Slot 'clearHistory'
        QtMocHelpers::SlotData<void()>(109, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'applyManualDeletion'
        QtMocHelpers::SlotData<void(const QVector<int> &)>(110, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 88, 111 },
        }}),
        // Slot 'undoLastFilter'
        QtMocHelpers::SlotData<void()>(112, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'resetCloud'
        QtMocHelpers::SlotData<void()>(113, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'consumeHardwarePackets'
        QtMocHelpers::SlotData<void()>(114, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMeshFinished'
        QtMocHelpers::SlotData<void()>(115, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFilterFinished'
        QtMocHelpers::SlotData<void()>(116, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onIcpFinished'
        QtMocHelpers::SlotData<void()>(117, 2, QMC::AccessPrivate, QMetaType::Void),
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
        case 6: _t->icpMergeFinished((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 7: _t->processingStarted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->processingFinished(); break;
        case 9: _t->requestClearVisualizer(); break;
        case 10: _t->mcuPacketReceived((*reinterpret_cast<std::add_pointer_t<quint32>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 11: _t->simProfileReceived((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<QPointF>>>(_a[2]))); break;
        case 12: _t->profileFrameReceived((*reinterpret_cast<std::add_pointer_t<ScanProfileFrame>>(_a[1]))); break;
        case 13: _t->pointCloudReady((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 14: _t->meshLoaded((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 15: _t->logMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 16: _t->triggerModeChanged((*reinterpret_cast<std::add_pointer_t<ScanTriggerMode>>(_a[1]))); break;
        case 17: _t->historySizeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->activeLayerChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->layersUpdated(); break;
        case 20: _t->connectMcu(); break;
        case 21: _t->disconnectMcu(); break;
        case 22: _t->connectLaser(); break;
        case 23: _t->connectLaserSim((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 24: _t->disconnectLaser(); break;
        case 25: _t->setDOffset((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 26: _t->setLateralOffset((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 27: _t->setResolution((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 28: _t->setRps((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 29: _t->setLaserProfileRate((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 30: _t->setLaserShutterUs((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 31: _t->setLaserAutoShutter((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 32: _t->setLaserMeasuringField((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 33: _t->setLaserPointsPerProfile((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 34: _t->setTriggerMode((*reinterpret_cast<std::add_pointer_t<ScanTriggerMode>>(_a[1]))); break;
        case 35: _t->setSerialPort((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 36: _t->onEncoderTrigger((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 37: _t->connectMcuSerial(); break;
        case 38: _t->disconnectMcuSerial(); break;
        case 39: _t->startScan((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 40: _t->startScan(); break;
        case 41: _t->stopScan(); break;
        case 42: _t->saveCurrentScan((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 43: { bool _r = _t->sendSerialCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 44: _t->sendZMove((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 45: _t->sendZHome(); break;
        case 46: _t->sendLinHome(); break;
        case 47: _t->mergeWithICPAsync((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3]))); break;
        case 48: _t->mergeSelectedLayers((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 49: _t->generateMeshAsync(); break;
        case 50: _t->clearMesh(); break;
        case 51: _t->setActiveLayer((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 52: _t->deleteLayer((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 53: _t->setLayerName((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 54: _t->setLayerZOffset((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 55: _t->addNewLayer((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 56: _t->addNewLayer((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 57: _t->applyFilterCylindrical((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[3]))); break;
        case 58: _t->applyFilterStatistical((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 59: _t->applyFilterRadius((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 60: _t->applyFilterVoxelGrid((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 61: _t->clearHistory(); break;
        case 62: _t->applyManualDeletion((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 63: _t->undoLastFilter(); break;
        case 64: _t->resetCloud(); break;
        case 65: _t->consumeHardwarePackets(); break;
        case 66: _t->onMeshFinished(); break;
        case 67: _t->onFilterFinished(); break;
        case 68: _t->onIcpFinished(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 48:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 62:
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
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QVector<QVector3D> & )>(_a, &ScanController::icpMergeFinished, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QString & )>(_a, &ScanController::processingStarted, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::processingFinished, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::requestClearVisualizer, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(quint32 , float )>(_a, &ScanController::mcuPacketReceived, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(float , const QVector<QPointF> & )>(_a, &ScanController::simProfileReceived, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const ScanProfileFrame & )>(_a, &ScanController::profileFrameReceived, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QVector<QVector3D> & )>(_a, &ScanController::pointCloudReady, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QVector<QVector3D> & )>(_a, &ScanController::meshLoaded, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(const QString & , const QString & )>(_a, &ScanController::logMessage, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(ScanTriggerMode )>(_a, &ScanController::triggerModeChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(int )>(_a, &ScanController::historySizeChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(int )>(_a, &ScanController::activeLayerChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::layersUpdated, 19))
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
        if (_id < 69)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 69;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 69)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 69;
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
void ScanController::icpMergeFinished(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void ScanController::processingStarted(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void ScanController::processingFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void ScanController::requestClearVisualizer()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void ScanController::mcuPacketReceived(quint32 _t1, float _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2);
}

// SIGNAL 11
void ScanController::simProfileReceived(float _t1, const QVector<QPointF> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2);
}

// SIGNAL 12
void ScanController::profileFrameReceived(const ScanProfileFrame & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void ScanController::pointCloudReady(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void ScanController::meshLoaded(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void ScanController::logMessage(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1, _t2);
}

// SIGNAL 16
void ScanController::triggerModeChanged(ScanTriggerMode _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void ScanController::historySizeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1);
}

// SIGNAL 18
void ScanController::activeLayerChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1);
}

// SIGNAL 19
void ScanController::layersUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}
QT_WARNING_POP
