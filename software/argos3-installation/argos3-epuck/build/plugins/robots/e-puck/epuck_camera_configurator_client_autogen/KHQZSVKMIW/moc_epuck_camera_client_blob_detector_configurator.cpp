/****************************************************************************
** Meta object code from reading C++ file 'epuck_camera_client_blob_detector_configurator.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../src/plugins/robots/e-puck/real_robot/vision/epuck_camera_client_blob_detector_configurator.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'epuck_camera_client_blob_detector_configurator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfiguratorTab_t {
    QByteArrayData data[12];
    char stringdata0[198];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfiguratorTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfiguratorTab_t qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfiguratorTab = {
    {
QT_MOC_LITERAL(0, 0, 52), // "argos::CEPuckCameraClientBlob..."
QT_MOC_LITERAL(1, 53, 15), // "checkSlidersMax"
QT_MOC_LITERAL(2, 69, 0), // ""
QT_MOC_LITERAL(3, 70, 15), // "checkSlidersMin"
QT_MOC_LITERAL(4, 86, 17), // "eyeDropperHandler"
QT_MOC_LITERAL(5, 104, 12), // "labelClicked"
QT_MOC_LITERAL(6, 117, 5), // "point"
QT_MOC_LITERAL(7, 123, 21), // "realImageStateHandler"
QT_MOC_LITERAL(8, 145, 5), // "state"
QT_MOC_LITERAL(9, 151, 18), // "enableColorHandler"
QT_MOC_LITERAL(10, 170, 21), // "updateEyeDropperRange"
QT_MOC_LITERAL(11, 192, 5) // "value"

    },
    "argos::CEPuckCameraClientBlobDetectorConfiguratorTab\0"
    "checkSlidersMax\0\0checkSlidersMin\0"
    "eyeDropperHandler\0labelClicked\0point\0"
    "realImageStateHandler\0state\0"
    "enableColorHandler\0updateEyeDropperRange\0"
    "value"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_argos__CEPuckCameraClientBlobDetectorConfiguratorTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    1,   52,    2, 0x08 /* Private */,
       7,    1,   55,    2, 0x08 /* Private */,
       9,    1,   58,    2, 0x08 /* Private */,
      10,    1,   61,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,   11,

       0        // eod
};

void argos::CEPuckCameraClientBlobDetectorConfiguratorTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CEPuckCameraClientBlobDetectorConfiguratorTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->checkSlidersMax(); break;
        case 1: _t->checkSlidersMin(); break;
        case 2: _t->eyeDropperHandler(); break;
        case 3: _t->labelClicked((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 4: _t->realImageStateHandler((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->enableColorHandler((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->updateEyeDropperRange((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject argos::CEPuckCameraClientBlobDetectorConfiguratorTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfiguratorTab.data,
    qt_meta_data_argos__CEPuckCameraClientBlobDetectorConfiguratorTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *argos::CEPuckCameraClientBlobDetectorConfiguratorTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *argos::CEPuckCameraClientBlobDetectorConfiguratorTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfiguratorTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int argos::CEPuckCameraClientBlobDetectorConfiguratorTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
struct qt_meta_stringdata_argos__MyLabel_t {
    QByteArrayData data[4];
    char stringdata0[28];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_argos__MyLabel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_argos__MyLabel_t qt_meta_stringdata_argos__MyLabel = {
    {
QT_MOC_LITERAL(0, 0, 14), // "argos::MyLabel"
QT_MOC_LITERAL(1, 15, 7), // "clicked"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 3) // "pos"

    },
    "argos::MyLabel\0clicked\0\0pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_argos__MyLabel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPoint,    3,

       0        // eod
};

void argos::MyLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MyLabel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->clicked((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MyLabel::*)(QPoint );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyLabel::clicked)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject argos::MyLabel::staticMetaObject = { {
    QMetaObject::SuperData::link<QLabel::staticMetaObject>(),
    qt_meta_stringdata_argos__MyLabel.data,
    qt_meta_data_argos__MyLabel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *argos::MyLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *argos::MyLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_argos__MyLabel.stringdata0))
        return static_cast<void*>(this);
    return QLabel::qt_metacast(_clname);
}

int argos::MyLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void argos::MyLabel::clicked(QPoint _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfigurator_t {
    QByteArrayData data[1];
    char stringdata0[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfigurator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfigurator_t qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfigurator = {
    {
QT_MOC_LITERAL(0, 0, 49) // "argos::CEPuckCameraClientBlob..."

    },
    "argos::CEPuckCameraClientBlobDetectorConfigurator"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_argos__CEPuckCameraClientBlobDetectorConfigurator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void argos::CEPuckCameraClientBlobDetectorConfigurator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject argos::CEPuckCameraClientBlobDetectorConfigurator::staticMetaObject = { {
    QMetaObject::SuperData::link<QTabWidget::staticMetaObject>(),
    qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfigurator.data,
    qt_meta_data_argos__CEPuckCameraClientBlobDetectorConfigurator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *argos::CEPuckCameraClientBlobDetectorConfigurator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *argos::CEPuckCameraClientBlobDetectorConfigurator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_argos__CEPuckCameraClientBlobDetectorConfigurator.stringdata0))
        return static_cast<void*>(this);
    return QTabWidget::qt_metacast(_clname);
}

int argos::CEPuckCameraClientBlobDetectorConfigurator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabWidget::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
