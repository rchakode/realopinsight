/****************************************************************************
** Meta object code from reading C++ file 'GraphView.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../client/src/GraphView.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GraphView.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GraphView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      39,   36,   10,   10, 0x05,
      76,   71,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     116,   10,   10,   10, 0x0a,
     126,   10,   10,   10, 0x0a,
     135,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GraphView[] = {
    "GraphView\0\0mouseIsOverNode(QString)\0"
    ",,\0expandNode(QString,bool,qint32)\0"
    ",pos\0rightClickOnItem(QGraphicsItem*,QPoint)\0"
    "capture()\0zoomIn()\0zoomOut()\0"
};

void GraphView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GraphView *_t = static_cast<GraphView *>(_o);
        switch (_id) {
        case 0: _t->mouseIsOverNode((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->expandNode((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< qint32(*)>(_a[3]))); break;
        case 2: _t->rightClickOnItem((*reinterpret_cast< QGraphicsItem*(*)>(_a[1])),(*reinterpret_cast< QPoint(*)>(_a[2]))); break;
        case 3: _t->capture(); break;
        case 4: _t->zoomIn(); break;
        case 5: _t->zoomOut(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GraphView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GraphView::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_GraphView,
      qt_meta_data_GraphView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GraphView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GraphView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GraphView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GraphView))
        return static_cast<void*>(const_cast< GraphView*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int GraphView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void GraphView::mouseIsOverNode(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GraphView::expandNode(QString _t1, bool _t2, qint32 _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GraphView::rightClickOnItem(QGraphicsItem * _t1, QPoint _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
