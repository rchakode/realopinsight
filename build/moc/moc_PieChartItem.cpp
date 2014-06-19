/****************************************************************************
** Meta object code from reading C++ file 'PieChartItem.hpp'
**
** Created: Fri Sep 6 00:55:32 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/PieChartItem.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PieChartItem.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PieChartItem[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   14,   13,   13, 0x0a,
      59,   45,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PieChartItem[] = {
    "PieChartItem\0\0rule\0setFillRule(Qt::FillRule)\0"
    "color1,color2\0setFillGradient(QColor,QColor)\0"
};

const QMetaObject PieChartItem::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PieChartItem,
      qt_meta_data_PieChartItem, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PieChartItem::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PieChartItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PieChartItem::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PieChartItem))
        return static_cast<void*>(const_cast< PieChartItem*>(this));
    return QWidget::qt_metacast(_clname);
}

int PieChartItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setFillRule((*reinterpret_cast< Qt::FillRule(*)>(_a[1]))); break;
        case 1: setFillGradient((*reinterpret_cast< const QColor(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
