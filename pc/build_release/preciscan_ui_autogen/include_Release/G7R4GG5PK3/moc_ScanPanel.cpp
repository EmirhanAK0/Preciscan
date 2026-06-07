/****************************************************************************
** Meta object code from reading C++ file 'ScanPanel.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../app/ui/panels/ScanPanel.hpp"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScanPanel.hpp' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9ScanPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto ScanPanel::qt_create_metaobjectdata<qt_meta_tag_ZN9ScanPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScanPanel",
        "mergedCloudReady",
        "",
        "QList<QVector3D>",
        "cloud",
        "onScanStarted",
        "onScanStopped",
        "onMcuConnected",
        "ok",
        "onLaserConnected",
        "addLayer",
        "points",
        "appendLog",
        "level",
        "msg",
        "onStartClicked",
        "onStopClicked",
        "onMergeClicked",
        "onDeleteLayer",
        "layerId",
        "onLayerZOffsetChanged",
        "mm"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'mergedCloudReady'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onScanStarted'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onScanStopped'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onMcuConnected'
        QtMocHelpers::SlotData<void(bool)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'onLaserConnected'
        QtMocHelpers::SlotData<void(bool)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'addLayer'
        QtMocHelpers::SlotData<void(const QVector<QVector3D> &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 11 },
        }}),
        // Slot 'appendLog'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 }, { QMetaType::QString, 14 },
        }}),
        // Slot 'onStartClicked'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStopClicked'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMergeClicked'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeleteLayer'
        QtMocHelpers::SlotData<void(int)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'onLayerZOffsetChanged'
        QtMocHelpers::SlotData<void(int, float)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Float, 21 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScanPanel, qt_meta_tag_ZN9ScanPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScanPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ScanPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ScanPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9ScanPanelE_t>.metaTypes,
    nullptr
} };

void ScanPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScanPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->mergedCloudReady((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 1: _t->onScanStarted(); break;
        case 2: _t->onScanStopped(); break;
        case 3: _t->onMcuConnected((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->onLaserConnected((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->addLayer((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        case 6: _t->appendLog((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->onStartClicked(); break;
        case 8: _t->onStopClicked(); break;
        case 9: _t->onMergeClicked(); break;
        case 10: _t->onDeleteLayer((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onLayerZOffsetChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScanPanel::*)(const QVector<QVector3D> & )>(_a, &ScanPanel::mergedCloudReady, 0))
            return;
    }
}

const QMetaObject *ScanPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScanPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ScanPanelE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ScanPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void ScanPanel::mergedCloudReady(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
