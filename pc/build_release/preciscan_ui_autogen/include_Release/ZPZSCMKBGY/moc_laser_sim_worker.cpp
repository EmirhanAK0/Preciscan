/****************************************************************************
** Meta object code from reading C++ file 'laser_sim_worker.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../sim/laser_sim_worker.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'laser_sim_worker.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3sim14LaserSimWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto sim::LaserSimWorker::qt_create_metaobjectdata<qt_meta_tag_ZN3sim14LaserSimWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "sim::LaserSimWorker",
        "profileReady",
        "",
        "theta_deg",
        "QList<QPointF>",
        "profile",
        "progressUpdated",
        "percent",
        "scanComplete",
        "QList<QVector3D>",
        "pointCloud"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'profileReady'
        QtMocHelpers::SignalData<void(float, const QVector<QPointF> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'progressUpdated'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Signal 'scanComplete'
        QtMocHelpers::SignalData<void(const QVector<QVector3D> &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LaserSimWorker, qt_meta_tag_ZN3sim14LaserSimWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject sim::LaserSimWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3sim14LaserSimWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3sim14LaserSimWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3sim14LaserSimWorkerE_t>.metaTypes,
    nullptr
} };

void sim::LaserSimWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LaserSimWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->profileReady((*reinterpret_cast<std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<QPointF>>>(_a[2]))); break;
        case 1: _t->progressUpdated((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->scanComplete((*reinterpret_cast<std::add_pointer_t<QList<QVector3D>>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LaserSimWorker::*)(float , const QVector<QPointF> & )>(_a, &LaserSimWorker::profileReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LaserSimWorker::*)(int )>(_a, &LaserSimWorker::progressUpdated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LaserSimWorker::*)(const QVector<QVector3D> & )>(_a, &LaserSimWorker::scanComplete, 2))
            return;
    }
}

const QMetaObject *sim::LaserSimWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *sim::LaserSimWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3sim14LaserSimWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int sim::LaserSimWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void sim::LaserSimWorker::profileReady(float _t1, const QVector<QPointF> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void sim::LaserSimWorker::progressUpdated(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void sim::LaserSimWorker::scanComplete(const QVector<QVector3D> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
