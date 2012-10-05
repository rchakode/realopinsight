/****************************************************************************
** Meta object code from reading C++ file 'MsgPanel.hpp'
**
** Created: Wed Sep 12 00:33:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/MsgPanel.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MsgPanel.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MsgPanel[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      31,    9,    9,    9, 0x0a,
      48,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MsgPanel[] = {
    "MsgPanel\0\0acknowledgeChanged()\0"
    "acknowledgeMsg()\0sortEventConsole()\0"
};

const QMetaObject MsgPanel::staticMetaObject = {
    { &QTableWidget::staticMetaObject, qt_meta_stringdata_MsgPanel,
      qt_meta_data_MsgPanel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MsgPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MsgPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MsgPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MsgPanel))
        return static_cast<void*>(const_cast< MsgPanel*>(this));
    return QTableWidget::qt_metacast(_clname);
}

int MsgPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: acknowledgeChanged(); break;
        case 1: acknowledgeMsg(); break;
        case 2: sortEventConsole(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void MsgPanel::acknowledgeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
