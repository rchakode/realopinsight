/****************************************************************************
** Meta object code from reading C++ file 'Preferences.hpp'
**
** Created: Fri Sep 6 00:55:32 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/Preferences.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Preferences.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ImageButton[] = {

 // content:
       5,       // revision
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

static const char qt_meta_stringdata_ImageButton[] = {
    "ImageButton\0"
};

const QMetaObject ImageButton::staticMetaObject = {
    { &QAbstractButton::staticMetaObject, qt_meta_stringdata_ImageButton,
      qt_meta_data_ImageButton, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ImageButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ImageButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ImageButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ImageButton))
        return static_cast<void*>(const_cast< ImageButton*>(this));
    return QAbstractButton::qt_metacast(_clname);
}

int ImageButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Preferences[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      33,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      62,   12,   12,   12, 0x0a,
      77,   12,   12,   12, 0x0a,
      93,   12,   12,   12, 0x0a,
     107,   12,   12,   12, 0x0a,
     122,   12,   12,   12, 0x0a,
     137,   12,   12,   12, 0x0a,
     152,  146,   12,   12, 0x0a,
     180,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Preferences[] = {
    "Preferences\0\0urlChanged(QString)\0"
    "sourcesChanged(QList<qint8>)\0"
    "handleCancel()\0applySettings()\0"
    "addAsSource()\0deleteSource()\0"
    "changePasswd()\0donate()\0state\0"
    "setAuthChainVisibility(int)\0"
    "handleSourceSelected()\0"
};

const QMetaObject Preferences::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Preferences,
      qt_meta_data_Preferences, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Preferences::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Preferences::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Preferences::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Preferences))
        return static_cast<void*>(const_cast< Preferences*>(this));
    return QDialog::qt_metacast(_clname);
}

int Preferences::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: urlChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: sourcesChanged((*reinterpret_cast< QList<qint8>(*)>(_a[1]))); break;
        case 2: handleCancel(); break;
        case 3: applySettings(); break;
        case 4: addAsSource(); break;
        case 5: deleteSource(); break;
        case 6: changePasswd(); break;
        case 7: donate(); break;
        case 8: setAuthChainVisibility((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 9: handleSourceSelected(); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Preferences::urlChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Preferences::sourcesChanged(QList<qint8> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
