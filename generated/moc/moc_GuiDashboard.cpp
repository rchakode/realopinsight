/****************************************************************************
** Meta object code from reading C++ file 'GuiDashboard.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../client/src/GuiDashboard.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GuiDashboard.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GuiDashboard[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      33,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      56,   13,   13,   13, 0x0a,
      74,   13,   13,   13, 0x0a,
     102,   13,   13,   13, 0x0a,
     154,  131,   13,   13, 0x0a,
     186,   13,   13,   13, 0x0a,
     214,  206,   13,   13, 0x0a,
     247,  241,   13,   13, 0x0a,
     283,   13,   13,   13, 0x0a,
     306,  206,   13,   13, 0x0a,
     342,  336,   13,   13, 0x0a,
     373,  364,   13,   13, 0x0a,
     397,  364,   13,   13, 0x0a,
     425,   13,   13,   13, 0x0a,
     448,  336,   13,   13, 0x0a,
     479,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GuiDashboard[] = {
    "GuiDashboard\0\0sortEventConsole()\0"
    "centralTabChanged(int)\0handleShowAbout()\0"
    "handleShowOnlineResources()\0"
    "handleChangePasswordAction()\0"
    "_nodeId,_expand,_level\0"
    "expandNode(QString,bool,qint32)\0"
    "centerGraphOnNode()\0_nodeId\0"
    "centerGraphOnNode(QString)\0_item\0"
    "centerGraphOnNode(QTreeWidgetItem*)\0"
    "filterNodeRelatedMsg()\0"
    "filterNodeRelatedMsg(QString)\0index\0"
    "handleTabChanged(int)\0_toggled\0"
    "toggleTroubleView(bool)\0"
    "toggleIncreaseMsgFont(bool)\0"
    "handleSettingsLoaded()\0"
    "handleSourceBxItemChanged(int)\0"
    "handleUpdateSourceUrl()\0"
};

void GuiDashboard::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GuiDashboard *_t = static_cast<GuiDashboard *>(_o);
        switch (_id) {
        case 0: _t->sortEventConsole(); break;
        case 1: _t->centralTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->handleShowAbout(); break;
        case 3: _t->handleShowOnlineResources(); break;
        case 4: _t->handleChangePasswordAction(); break;
        case 5: _t->expandNode((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2])),(*reinterpret_cast< const qint32(*)>(_a[3]))); break;
        case 6: _t->centerGraphOnNode(); break;
        case 7: _t->centerGraphOnNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->centerGraphOnNode((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 9: _t->filterNodeRelatedMsg(); break;
        case 10: _t->filterNodeRelatedMsg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->handleTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->toggleTroubleView((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->toggleIncreaseMsgFont((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->handleSettingsLoaded(); break;
        case 15: _t->handleSourceBxItemChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->handleUpdateSourceUrl(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GuiDashboard::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GuiDashboard::staticMetaObject = {
    { &DashboardBase::staticMetaObject, qt_meta_stringdata_GuiDashboard,
      qt_meta_data_GuiDashboard, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GuiDashboard::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GuiDashboard::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GuiDashboard::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GuiDashboard))
        return static_cast<void*>(const_cast< GuiDashboard*>(this));
    return DashboardBase::qt_metacast(_clname);
}

int GuiDashboard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DashboardBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void GuiDashboard::sortEventConsole()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void GuiDashboard::centralTabChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
