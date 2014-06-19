/****************************************************************************
** Meta object code from reading C++ file 'SvNavigatorTree.hpp'
**
** Created: Wed May 15 21:08:12 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/SvNavigatorTree.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SvNavigatorTree.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SvNavigatorTree[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   17,   16,   16, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_SvNavigatorTree[] = {
    "SvNavigatorTree\0\0_nodeId\0"
    "treeNodeMoved(QString)\0"
};

const QMetaObject SvNavigatorTree::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_SvNavigatorTree,
      qt_meta_data_SvNavigatorTree, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SvNavigatorTree::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SvNavigatorTree::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SvNavigatorTree::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SvNavigatorTree))
        return static_cast<void*>(const_cast< SvNavigatorTree*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int SvNavigatorTree::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: treeNodeMoved((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void SvNavigatorTree::treeNodeMoved(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
