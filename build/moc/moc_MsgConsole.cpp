/****************************************************************************
** Meta object code from reading C++ file 'MsgConsole.hpp'
**
** Created: Wed Apr 3 21:16:48 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/MsgConsole.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MsgConsole.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MsgConsoleProxyModel[] = {

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

static const char qt_meta_stringdata_MsgConsoleProxyModel[] = {
    "MsgConsoleProxyModel\0"
};

const QMetaObject MsgConsoleProxyModel::staticMetaObject = {
    { &QSortFilterProxyModel::staticMetaObject, qt_meta_stringdata_MsgConsoleProxyModel,
      qt_meta_data_MsgConsoleProxyModel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MsgConsoleProxyModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MsgConsoleProxyModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MsgConsoleProxyModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MsgConsoleProxyModel))
        return static_cast<void*>(const_cast< MsgConsoleProxyModel*>(this));
    return QSortFilterProxyModel::qt_metacast(_clname);
}

int MsgConsoleProxyModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSortFilterProxyModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_MsgConsole[] = {

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
      12,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   11,   11,   11, 0x0a,
      50,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MsgConsole[] = {
    "MsgConsole\0\0acknowledgeChanged()\0"
    "acknowledgeMsg()\0sortEventConsole()\0"
};

const QMetaObject MsgConsole::staticMetaObject = {
    { &QTableView::staticMetaObject, qt_meta_stringdata_MsgConsole,
      qt_meta_data_MsgConsole, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MsgConsole::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MsgConsole::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MsgConsole::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MsgConsole))
        return static_cast<void*>(const_cast< MsgConsole*>(this));
    return QTableView::qt_metacast(_clname);
}

int MsgConsole::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableView::qt_metacall(_c, _id, _a);
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
void MsgConsole::acknowledgeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
