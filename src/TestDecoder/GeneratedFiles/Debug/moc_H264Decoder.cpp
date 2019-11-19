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
    QByteArrayData data[9];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_H264Decoder_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_H264Decoder_t qt_meta_stringdata_H264Decoder = {
    {
QT_MOC_LITERAL(0, 0, 11), // "H264Decoder"
QT_MOC_LITERAL(1, 12, 24), // "on_createsession_clicked"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 24), // "on_deletesession_clicked"
QT_MOC_LITERAL(4, 63, 19), // "on_trydemux_clicked"
QT_MOC_LITERAL(5, 83, 28), // "on_trynetstreamdemux_clicked"
QT_MOC_LITERAL(6, 112, 22), // "on_begindecode_clicked"
QT_MOC_LITERAL(7, 135, 21), // "on_stopdecode_clicked"
QT_MOC_LITERAL(8, 157, 18) // "on_GetFree_clicked"

    },
    "H264Decoder\0on_createsession_clicked\0"
    "\0on_deletesession_clicked\0on_trydemux_clicked\0"
    "on_trynetstreamdemux_clicked\0"
    "on_begindecode_clicked\0on_stopdecode_clicked\0"
    "on_GetFree_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_H264Decoder[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x0a /* Public */,
       3,    0,   50,    2, 0x0a /* Public */,
       4,    0,   51,    2, 0x0a /* Public */,
       5,    0,   52,    2, 0x0a /* Public */,
       6,    0,   53,    2, 0x0a /* Public */,
       7,    0,   54,    2, 0x0a /* Public */,
       8,    0,   55,    2, 0x0a /* Public */,

 // slots: parameters
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
        case 0: _t->on_createsession_clicked(); break;
        case 1: _t->on_deletesession_clicked(); break;
        case 2: _t->on_trydemux_clicked(); break;
        case 3: _t->on_trynetstreamdemux_clicked(); break;
        case 4: _t->on_begindecode_clicked(); break;
        case 5: _t->on_stopdecode_clicked(); break;
        case 6: _t->on_GetFree_clicked(); break;
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
