/****************************************************************************
** Meta object code from reading C++ file 'epuck_camera_client_main_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../src/plugins/robots/e-puck/real_robot/vision/epuck_camera_client_main_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'epuck_camera_client_main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_argos__CEPuckCameraClientMainWindow_t {
    QByteArrayData data[8];
    char stringdata0[124];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_argos__CEPuckCameraClientMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_argos__CEPuckCameraClientMainWindow_t qt_meta_stringdata_argos__CEPuckCameraClientMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 35), // "argos::CEPuckCameraClientMain..."
QT_MOC_LITERAL(1, 36, 13), // "GetAttributes"
QT_MOC_LITERAL(2, 50, 0), // ""
QT_MOC_LITERAL(3, 51, 13), // "SetAttributes"
QT_MOC_LITERAL(4, 65, 7), // "Refresh"
QT_MOC_LITERAL(5, 73, 17), // "SaveConfiguration"
QT_MOC_LITERAL(6, 91, 19), // "RefreshTimerTimeout"
QT_MOC_LITERAL(7, 111, 12) // "PauseChecked"

    },
    "argos::CEPuckCameraClientMainWindow\0"
    "GetAttributes\0\0SetAttributes\0Refresh\0"
    "SaveConfiguration\0RefreshTimerTimeout\0"
    "PauseChecked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_argos__CEPuckCameraClientMainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x08 /* Private */,
       3,    0,   45,    2, 0x08 /* Private */,
       4,    0,   46,    2, 0x08 /* Private */,
       5,    0,   47,    2, 0x08 /* Private */,
       6,    0,   48,    2, 0x08 /* Private */,
       7,    0,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void argos::CEPuckCameraClientMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CEPuckCameraClientMainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->GetAttributes(); break;
        case 1: _t->SetAttributes(); break;
        case 2: _t->Refresh(); break;
        case 3: _t->SaveConfiguration(); break;
        case 4: _t->RefreshTimerTimeout(); break;
        case 5: _t->PauseChecked(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject argos::CEPuckCameraClientMainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_argos__CEPuckCameraClientMainWindow.data,
    qt_meta_data_argos__CEPuckCameraClientMainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *argos::CEPuckCameraClientMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *argos::CEPuckCameraClientMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_argos__CEPuckCameraClientMainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int argos::CEPuckCameraClientMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
