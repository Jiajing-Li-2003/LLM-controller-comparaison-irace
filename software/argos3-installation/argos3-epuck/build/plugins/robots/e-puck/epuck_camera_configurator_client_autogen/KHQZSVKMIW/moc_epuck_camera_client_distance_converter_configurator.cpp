/****************************************************************************
** Meta object code from reading C++ file 'epuck_camera_client_distance_converter_configurator.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../src/plugins/robots/e-puck/real_robot/vision/epuck_camera_client_distance_converter_configurator.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'epuck_camera_client_distance_converter_configurator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_argos__CEPuckCameraClientDistanceConverterConfigurator_t {
    QByteArrayData data[4];
    char stringdata0[88];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_argos__CEPuckCameraClientDistanceConverterConfigurator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_argos__CEPuckCameraClientDistanceConverterConfigurator_t qt_meta_stringdata_argos__CEPuckCameraClientDistanceConverterConfigurator = {
    {
QT_MOC_LITERAL(0, 0, 54), // "argos::CEPuckCameraClientDist..."
QT_MOC_LITERAL(1, 55, 16), // "ComputeVariables"
QT_MOC_LITERAL(2, 72, 0), // ""
QT_MOC_LITERAL(3, 73, 14) // "ResetVariables"

    },
    "argos::CEPuckCameraClientDistanceConverterConfigurator\0"
    "ComputeVariables\0\0ResetVariables"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_argos__CEPuckCameraClientDistanceConverterConfigurator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x0a /* Public */,
       3,    0,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void argos::CEPuckCameraClientDistanceConverterConfigurator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CEPuckCameraClientDistanceConverterConfigurator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->ComputeVariables(); break;
        case 1: _t->ResetVariables(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject argos::CEPuckCameraClientDistanceConverterConfigurator::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_argos__CEPuckCameraClientDistanceConverterConfigurator.data,
    qt_meta_data_argos__CEPuckCameraClientDistanceConverterConfigurator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *argos::CEPuckCameraClientDistanceConverterConfigurator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *argos::CEPuckCameraClientDistanceConverterConfigurator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_argos__CEPuckCameraClientDistanceConverterConfigurator.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int argos::CEPuckCameraClientDistanceConverterConfigurator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
