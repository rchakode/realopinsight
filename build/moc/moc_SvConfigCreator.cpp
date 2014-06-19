/****************************************************************************
** Meta object code from reading C++ file 'SvConfigCreator.hpp'
**
** Created: Wed Apr 3 21:16:37 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/SvConfigCreator.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SvConfigCreator.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SvCreator[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x0a,
      21,   10,   10,   10, 0x0a,
      36,   31,   10,   10, 0x0a,
      62,   10,   10,   10, 0x0a,
      77,   10,   10,   10, 0x0a,
      97,   10,   10,   10, 0x0a,
     110,   10,   10,   10, 0x0a,
     130,   10,   10,   10, 0x0a,
     137,   10,   10,   10, 0x0a,
     144,   10,   10,   10, 0x0a,
     157,   10,  153,   10, 0x0a,
     180,   10,  153,   10, 0x2a,
     199,   10,   10,   10, 0x0a,
     239,   10,   10,   10, 0x0a,
     261,   10,   10,   10, 0x0a,
     289,   10,   10,   10, 0x0a,
     318,   10,   10,   10, 0x0a,
     350,   10,   10,   10, 0x0a,
     378,   10,   10,   10, 0x0a,
     396,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SvCreator[] = {
    "SvCreator\0\0newView()\0newNode()\0node\0"
    "insertFromSelected(NodeT)\0copySelected()\0"
    "pasteFromSelected()\0deleteNode()\0"
    "deleteNode(QString)\0open()\0save()\0"
    "saveAs()\0int\0treatCloseAction(bool)\0"
    "treatCloseAction()\0"
    "fillEditorFromService(QTreeWidgetItem*)\0"
    "handleReturnPressed()\0handleSelectedNodeChanged()\0"
    "handleTreeNodeMoved(QString)\0"
    "handleNodeTypeActivated(qint32)\0"
    "handleShowOnlineResources()\0"
    "handleShowAbout()\0import()\0"
};

const QMetaObject SvCreator::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SvCreator,
      qt_meta_data_SvCreator, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SvCreator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SvCreator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SvCreator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SvCreator))
        return static_cast<void*>(const_cast< SvCreator*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int SvCreator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newView(); break;
        case 1: newNode(); break;
        case 2: insertFromSelected((*reinterpret_cast< const NodeT(*)>(_a[1]))); break;
        case 3: copySelected(); break;
        case 4: pasteFromSelected(); break;
        case 5: deleteNode(); break;
        case 6: deleteNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: open(); break;
        case 8: save(); break;
        case 9: saveAs(); break;
        case 10: { int _r = treatCloseAction((*reinterpret_cast< const bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 11: { int _r = treatCloseAction();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 12: fillEditorFromService((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 13: handleReturnPressed(); break;
        case 14: handleSelectedNodeChanged(); break;
        case 15: handleTreeNodeMoved((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 16: handleNodeTypeActivated((*reinterpret_cast< qint32(*)>(_a[1]))); break;
        case 17: handleShowOnlineResources(); break;
        case 18: handleShowAbout(); break;
        case 19: import(); break;
        default: ;
        }
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
