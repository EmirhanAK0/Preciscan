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
        "triggerSourceChanged",
        "TriggerSource",
        "src",
        "historySizeChanged",
        "size",
        "activeLayerChanged",
        "id",
        "layersUpdated",
        "autoCalibrationFinished",
        "bestOffset",
        "score",
        "calibration3DFinished",
        "success",
        "report",
        "connectMcu",
        "disconnectMcu",
        "connectLaser",
        "connectLaserSim",
        "stlPath",
        "disconnectLaser",
        "setDOffset",
        "mm",
        "setLateralOffset",
        "setAs5600Resolution",
        "deg",
        "setStepResolution",
        "setSecPerRev",
        "sec",
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
        "setTriggerSource",
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
        "icpMode",
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
        "getCalibrator",
        "AutoCalibrator*",
        "updateActiveCalibration",
        "filePath",
        "disableCalibration",
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
        "beginManualAlignment",
        "updateManualAlignment",
        "tx",
        "ty",
        "tz",
        "rx",
        "ry",
        "rz",
        "commitManualAlignment",
        "cancelManualAlignment",
        "autoCalibrateOffsetAsync",
        "start3DCalibrationAsync",
        "method",
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
        // Signal 'triggerSourceChanged'
        QtMocHelpers::SignalData<void(TriggerSource)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 39, 40 },
        }}),
        // Signal 'historySizeChanged'
        QtMocHelpers::SignalData<void(int)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 42 },
        }}),
        // Signal 'activeLayerChanged'
        QtMocHelpers::SignalData<void(int)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 44 },
        }}),
        // Signal 'layersUpdated'
        QtMocHelpers::SignalData<void()>(45, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoCalibrationFinished'
        QtMocHelpers::SignalData<void(float, double)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 47 }, { QMetaType::Double, 48 },
        }}),
        // Signal 'calibration3DFinished'
        QtMocHelpers::SignalData<void(bool, QString)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 50 }, { QMetaType::QString, 51 },
        }}),
        // Slot 'connectMcu'
        QtMocHelpers::SlotData<void()>(52, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'disconnectMcu'
        QtMocHelpers::SlotData<void()>(53, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectLaser'
        QtMocHelpers::SlotData<void()>(54, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'connectLaserSim'
        QtMocHelpers::SlotData<void(const QString &)>(55, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 56 },
        }}),
        // Slot 'disconnectLaser'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setDOffset'
        QtMocHelpers::SlotData<void(float)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 59 },
        }}),
        // Slot 'setLateralOffset'
        QtMocHelpers::SlotData<void(float)>(60, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 59 },
        }}),
        // Slot 'setAs5600Resolution'
        QtMocHelpers::SlotData<void(float)>(61, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 62 },
        }}),
        // Slot 'setStepResolution'
        QtMocHelpers::SlotData<void(float)>(63, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 62 },
        }}),
        // Slot 'setSecPerRev'
        QtMocHelpers::SlotData<void(float)>(64, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 65 },
        }}),
        // Slot 'setLaserProfileRate'
        QtMocHelpers::SlotData<void(int)>(66, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 67 },
        }}),
        // Slot 'setLaserShutterUs'
        QtMocHelpers::SlotData<void(int)>(68, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 69 },
        }}),
        // Slot 'setLaserAutoShutter'
        QtMocHelpers::SlotData<void(bool)>(70, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 71 },
        }}),
        // Slot 'setLaserMeasuringField'
        QtMocHelpers::SlotData<void(const QString &)>(72, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 73 },
        }}),
        // Slot 'setLaserPointsPerProfile'
        QtMocHelpers::SlotData<void(int)>(74, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 75 },
        }}),
        // Slot 'setTriggerMode'
        QtMocHelpers::SlotData<void(ScanTriggerMode)>(76, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 36, 37 },
        }}),
        // Slot 'setTriggerSource'
        QtMocHelpers::SlotData<void(TriggerSource)>(77, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 39, 40 },
        }}),
        // Slot 'setSerialPort'
        QtMocHelpers::SlotData<void(const QString &)>(78, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 79 },
        }}),
        // Slot 'onEncoderTrigger'
        QtMocHelpers::SlotData<void(float)>(80, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 81 },
        }}),
        // Slot 'connectMcuSerial'
        QtMocHelpers::SlotData<void()>(82, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'disconnectMcuSerial'
        QtMocHelpers::SlotData<void()>(83, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startScan'
        QtMocHelpers::SlotData<void(int)>(84, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 85 },
        }}),
        // Slot 'startScan'
        QtMocHelpers::SlotData<void()>(84, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'stopScan'
        QtMocHelpers::SlotData<void()>(86, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'saveCurrentScan'
        QtMocHelpers::SlotData<void(const QString &)>(87, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 88 },
        }}),
        // Slot 'sendSerialCommand'
        QtMocHelpers::SlotData<bool(const QString &)>(89, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 90 },
        }}),
        // Slot 'sendZMove'
        QtMocHelpers::SlotData<void(float)>(91, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 59 },
        }}),
        // Slot 'sendZHome'
        QtMocHelpers::SlotData<void()>(92, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'sendLinHome'
        QtMocHelpers::SlotData<void()>(93, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'mergeWithICPAsync'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &, const QVector<QVector3D> &, int)>(94, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 95 }, { 0x80000000 | 12, 96 }, { QMetaType::Int, 97 },
        }}),
        // Slot 'mergeSelectedLayers'
        QtMocHelpers::SlotData<void(const QVector<int> &, const QString &)>(98, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 99, 100 }, { QMetaType::QString, 37 },
        }}),
        // Slot 'generateMeshAsync'
        QtMocHelpers::SlotData<void()>(101, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'clearMesh'
        QtMocHelpers::SlotData<void()>(102, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setActiveLayer'
        QtMocHelpers::SlotData<void(int)>(103, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 44 },
        }}),
        // Slot 'deleteLayer'
        QtMocHelpers::SlotData<void(int)>(104, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 44 },
        }}),
        // Slot 'setLayerName'
        QtMocHelpers::SlotData<void(int, const QString &)>(105, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 44 }, { QMetaType::QString, 106 },
        }}),
        // Slot 'setLayerZOffset'
        QtMocHelpers::SlotData<void(int, float)>(107, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 44 }, { QMetaType::Float, 59 },
        }}),
        // Slot 'addNewLayer'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &, const QString &)>(108, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 75 }, { QMetaType::QString, 106 },
        }}),
        // Slot 'addNewLayer'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &)>(108, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 12, 75 },
        }}),
        // Slot 'getCalibrator'
        QtMocHelpers::SlotData<AutoCalibrator *() const>(109, 2, QMC::AccessPublic, 0x80000000 | 110),
        // Slot 'updateActiveCalibration'
        QtMocHelpers::SlotData<void(QString)>(111, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 112 },
        }}),
        // Slot 'disableCalibration'
        QtMocHelpers::SlotData<void()>(113, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'applyFilterCylindrical'
        QtMocHelpers::SlotData<void(float, float, float)>(114, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 115 }, { QMetaType::Float, 116 }, { QMetaType::Float, 117 },
        }}),
        // Slot 'applyFilterStatistical'
        QtMocHelpers::SlotData<void(int, float)>(118, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 119 }, { QMetaType::Float, 120 },
        }}),
        // Slot 'applyFilterRadius'
        QtMocHelpers::SlotData<void(float, int)>(121, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 115 }, { QMetaType::Int, 122 },
        }}),
        // Slot 'applyFilterVoxelGrid'
        QtMocHelpers::SlotData<void(float)>(123, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 124 },
        }}),
        // Slot 'clearHistory'
        QtMocHelpers::SlotData<void()>(125, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'applyManualDeletion'
        QtMocHelpers::SlotData<void(const QVector<int> &)>(126, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 99, 127 },
        }}),
        // Slot 'undoLastFilter'
        QtMocHelpers::SlotData<void()>(128, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'resetCloud'
        QtMocHelpers::SlotData<void()>(129, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'beginManualAlignment'
        QtMocHelpers::SlotData<void()>(130, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateManualAlignment'
        QtMocHelpers::SlotData<void(float, float, float, float, float, float)>(131, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 132 }, { QMetaType::Float, 133 }, { QMetaType::Float, 134 }, { QMetaType::Float, 135 },
            { QMetaType::Float, 136 }, { QMetaType::Float, 137 },
        }}),
        // Slot 'commitManualAlignment'
        QtMocHelpers::SlotData<void()>(138, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'cancelManualAlignment'
        QtMocHelpers::SlotData<void()>(139, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'autoCalibrateOffsetAsync'
        QtMocHelpers::SlotData<void()>(140, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'start3DCalibrationAsync'
        QtMocHelpers::SlotData<void(int)>(141, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 142 },
        }}),
        // Slot 'start3DCalibrationAsync'
        QtMocHelpers::SlotData<void()>(141, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'consumeHardwarePackets'
        QtMocHelpers::SlotData<void()>(143, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMeshFinished'
        QtMocHelpers::SlotData<void()>(144, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFilterFinished'
        QtMocHelpers::SlotData<void()>(145, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onIcpFinished'
        QtMocHelpers::SlotData<void()>(146, 2, QMC::AccessPrivate, QMetaType::Void),
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
        case 17: _t->triggerSourceChanged((*reinterpret_cast<std::add_pointer_t<TriggerSource>>(_a[1]))); break;
        case 18: _t->historySizeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->activeLayerChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->layersUpdated(); break;
        case 21: _t->autoCalibrationFinished((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 22: _t->calibration3DFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 23: _t->connectMcu(); break;
        case 24: _t->disconnectMcu(); break;
        case 25: _t->connectLaser(); break;
        case 26: _t->connectLaserSim((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 27: _t->disconnectLaser(); break;
        case 28: _t->setDOffset((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 29: _t->setLateralOffset((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 30: _t->setAs5600Resolution((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 31: _t->setStepResolution((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 32: _t->setSecPerRev((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 33: _t->setLaserProfileRate((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 34: _t->setLaserShutterUs((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 35: _t->setLaserAutoShutter((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 36: _t->setLaserMeasuringField((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 37: _t->setLaserPointsPerProfile((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 38: _t->setTriggerMode((*reinterpret_cast<std::add_pointer_t<ScanTriggerMode>>(_a[1]))); break;
        case 39: _t->setTriggerSource((*reinterpret_cast<std::add_pointer_t<TriggerSource>>(_a[1]))); break;
        case 40: _t->setSerialPort((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 41: _t->onEncoderTrigger((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 42: _t->connectMcuSerial(); break;
        case 43: _t->disconnectMcuSerial(); break;
        case 44: _t->startScan((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 45: _t->startScan(); break;
        case 46: _t->stopScan(); break;
        case 47: _t->saveCurrentScan((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 48: { bool _r = _t->sendSerialCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 49: _t->sendZMove((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 50: _t->sendZHome(); break;
        case 51: _t->sendLinHome(); break;
        case 52: _t->mergeWithICPAsync((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 53: _t->mergeSelectedLayers((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 54: _t->generateMeshAsync(); break;
        case 55: _t->clearMesh(); break;
        case 56: _t->setActiveLayer((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 57: _t->deleteLayer((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 58: _t->setLayerName((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 59: _t->setLayerZOffset((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 60: _t->addNewLayer((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 61: _t->addNewLayer((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 62: { AutoCalibrator* _r = _t->getCalibrator();
            if (_a[0]) *reinterpret_cast<AutoCalibrator**>(_a[0]) = std::move(_r); }  break;
        case 63: _t->updateActiveCalibration((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 64: _t->disableCalibration(); break;
        case 65: _t->applyFilterCylindrical((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[3]))); break;
        case 66: _t->applyFilterStatistical((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        case 67: _t->applyFilterRadius((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 68: _t->applyFilterVoxelGrid((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 69: _t->clearHistory(); break;
        case 70: _t->applyManualDeletion((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 71: _t->undoLastFilter(); break;
        case 72: _t->resetCloud(); break;
        case 73: _t->beginManualAlignment(); break;
        case 74: _t->updateManualAlignment((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[6]))); break;
        case 75: _t->commitManualAlignment(); break;
        case 76: _t->cancelManualAlignment(); break;
        case 77: _t->autoCalibrateOffsetAsync(); break;
        case 78: _t->start3DCalibrationAsync((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 79: _t->start3DCalibrationAsync(); break;
        case 80: _t->consumeHardwarePackets(); break;
        case 81: _t->onMeshFinished(); break;
        case 82: _t->onFilterFinished(); break;
        case 83: _t->onIcpFinished(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 53:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 70:
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
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(TriggerSource )>(_a, &ScanController::triggerSourceChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(int )>(_a, &ScanController::historySizeChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(int )>(_a, &ScanController::activeLayerChanged, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)()>(_a, &ScanController::layersUpdated, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(float , double )>(_a, &ScanController::autoCalibrationFinished, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScanController::*)(bool , QString )>(_a, &ScanController::calibration3DFinished, 22))
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
        if (_id < 84)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 84;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 84)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 84;
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
void ScanController::triggerSourceChanged(TriggerSource _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1);
}

// SIGNAL 18
void ScanController::historySizeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1);
}

// SIGNAL 19
void ScanController::activeLayerChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1);
}

// SIGNAL 20
void ScanController::layersUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 20, nullptr);
}

// SIGNAL 21
void ScanController::autoCalibrationFinished(float _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1, _t2);
}

// SIGNAL 22
void ScanController::calibration3DFinished(bool _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1, _t2);
}
QT_WARNING_POP
