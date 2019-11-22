/****************************************************************************
** Meta object code from reading C++ file 'H264Decoder.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../H264Decoder.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'H264Decoder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_H264Decoder_t {
    QByteArrayData data[12];
    char stringdata0[253];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_H264Decoder_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_H264Decoder_t qt_meta_stringdata_H264Decoder = {
    {
QT_MOC_LITERAL(0, 0, 11), // "H264Decoder"
QT_MOC_LITERAL(1, 12, 24), // "on_CreateSession_clicked"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 24), // "on_DeleteSession_clicked"
QT_MOC_LITERAL(4, 63, 28), // "on_TryBitStreamDemux_clicked"
QT_MOC_LITERAL(5, 92, 28), // "on_TryNetStreamDemux_clicked"
QT_MOC_LITERAL(6, 121, 22), // "on_BeginDecode_clicked"
QT_MOC_LITERAL(7, 144, 21), // "on_StopDecode_clicked"
QT_MOC_LITERAL(8, 166, 18), // "on_GetFree_clicked"
QT_MOC_LITERAL(9, 185, 19), // "on_OpenFile_clicked"
QT_MOC_LITERAL(10, 205, 24), // "on_StartSendData_clicked"
QT_MOC_LITERAL(11, 230, 22) // "on_EndSendData_clicked"

    },
    "H264Decoder\0on_CreateSession_clicked\0"
    "\0on_DeleteSession_clicked\0"
    "on_TryBitStreamDemux_clicked\0"
    "on_TryNetStreamDemux_clicked\0"
    "on_BeginDecode_clicked\0on_StopDecode_clicked\0"
    "on_GetFree_clicked\0on_OpenFile_clicked\0"
    "on_StartSendData_clicked\0"
    "on_EndSendData_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_H264Decoder[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    0,   65,    2, 0x0a /* Public */,
       4,    0,   66,    2, 0x0a /* Public */,
       5,    0,   67,    2, 0x0a /* Public */,
       6,    0,   68,    2, 0x0a /* Public */,
       7,    0,   69,    2, 0x0a /* Public */,
       8,    0,   70,    2, 0x0a /* Public */,
       9,    0,   71,    2, 0x0a /* Public */,
      10,    0,   72,    2, 0x0a /* Public */,
      11,    0,   73,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void H264Decoder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        H264Decoder *_t = static_cast<H264Decoder *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_CreateSession_clicked(); break;
        case 1: _t->on_DeleteSession_clicked(); break;
        case 2: _t->on_TryBitStreamDemux_clicked(); break;
        case 3: _t->on_TryNetStreamDemux_clicked(); break;
        case 4: _t->on_BeginDecode_clicked(); break;
        case 5: _t->on_StopDecode_clicked(); break;
        case 6: _t->on_GetFree_clicked(); break;
        case 7: _t->on_OpenFile_clicked(); break;
        case 8: _t->on_StartSendData_clicked(); break;
        case 9: _t->on_EndSendData_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject H264Decoder::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_H264Decoder.data,
      qt_meta_data_H264Decoder,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *H264Decoder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *H264Decoder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_H264Decoder.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int H264Decoder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
