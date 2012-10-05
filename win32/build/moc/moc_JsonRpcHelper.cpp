/****************************************************************************
** Meta object code from reading C++ file 'JsonRpcHelper.hpp'
**
** Created: Tue Aug 7 19:47:19 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/JsonRpcHelper.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'JsonRpcHelper.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_JsonRpcHelper[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   15,   14,   14, 0x0a,
      45,   14,   14,   14, 0x0a,
      64,   54,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_JsonRpcHelper[] = {
    "JsonRpcHelper\0\0reply\0process(QNetworkReply*)\0"
    "submit()\0user,pass\0login(QString,QString)\0"
};

const QMetaObject JsonRpcHelper::staticMetaObject = {
    { &QNetworkAccessManager::staticMetaObject, qt_meta_stringdata_JsonRpcHelper,
      qt_meta_data_JsonRpcHelper, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &JsonRpcHelper::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *JsonRpcHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *JsonRpcHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_JsonRpcHelper))
        return static_cast<void*>(const_cast< JsonRpcHelper*>(this));
    return QNetworkAccessManager::qt_metacast(_clname);
}

int JsonRpcHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QNetworkAccessManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: process((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 1: submit(); break;
        case 2: login((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
