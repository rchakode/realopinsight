/****************************************************************************
** Meta object code from reading C++ file 'DashboardBase.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../core/src/DashboardBase.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DashboardBase.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DashboardBase[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      38,   34,   14,   14, 0x05,
      63,   14,   14,   14, 0x05,
      80,   14,   14,   14, 0x05,
     107,   98,   14,   14, 0x05,
     136,   34,   14,   14, 0x05,
     159,   14,   14,   14, 0x05,
     183,   14,   14,   14, 0x05,
     202,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     219,   14,   14,   14, 0x0a,
     236,  232,   14,   14, 0x0a,
     257,  232,   14,   14, 0x0a,
     283,  232,   14,   14, 0x0a,
     312,   14,   14,   14, 0x0a,
     328,  232,   14,   14, 0x0a,
     377,  371,  351,   14, 0x0a,
     408,  232,  403,   14, 0x0a,
     441,  437,   14,   14, 0x0a,
     483,   34,   14,   14, 0x0a,
     526,  512,   14,   14, 0x0a,
     562,   14,  551,   14, 0x0a,
     577,   14,  570,   14, 0x0a,
     588,   14,  403,   14, 0x0a,
     613,  607,   14,   14, 0x0a,
     651,   14,  639,   14, 0x0a,
     665,   14,  661,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DashboardBase[] = {
    "DashboardBase\0\0sortEventConsole()\0msg\0"
    "updateStatusBar(QString)\0settingsLoaded()\0"
    "updateSourceUrl()\0interval\0"
    "timerIntervalChanged(qint32)\0"
    "errorOccurred(QString)\0dashboardLinkSelected()\0"
    "updateInprogress()\0updateFinished()\0"
    "runMonitor()\0src\0runMonitor(SourceT&)\0"
    "runNgrt4ndUpdate(SourceT)\0"
    "runLivestatusUpdate(SourceT)\0"
    "resetStatData()\0prepareUpdate(SourceT)\0"
    "SeverityWeightInfoT\0_node\0"
    "updateNodeStates(QString)\0bool\0"
    "allocSourceHandler(SourceT&)\0ids\0"
    "handleSourceSettingsChanged(QList<qint8>)\0"
    "handleErrorOccurred(QString)\0preferencePtr\0"
    "initialize(Preferences*)\0CoreDataT*\0"
    "cdata()\0qint32\0userRole()\0showOnlyTroubles()\0"
    "value\0setShowOnlyTroubles(bool)\0"
    "SourceListT\0sources()\0int\0firstSrcIndex()\0"
};

void DashboardBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DashboardBase *_t = static_cast<DashboardBase *>(_o);
        switch (_id) {
        case 0: _t->sortEventConsole(); break;
        case 1: _t->updateStatusBar((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->settingsLoaded(); break;
        case 3: _t->updateSourceUrl(); break;
        case 4: _t->timerIntervalChanged((*reinterpret_cast< qint32(*)>(_a[1]))); break;
        case 5: _t->errorOccurred((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->dashboardLinkSelected(); break;
        case 7: _t->updateInprogress(); break;
        case 8: _t->updateFinished(); break;
        case 9: _t->runMonitor(); break;
        case 10: _t->runMonitor((*reinterpret_cast< SourceT(*)>(_a[1]))); break;
        case 11: _t->runNgrt4ndUpdate((*reinterpret_cast< const SourceT(*)>(_a[1]))); break;
        case 12: _t->runLivestatusUpdate((*reinterpret_cast< const SourceT(*)>(_a[1]))); break;
        case 13: _t->resetStatData(); break;
        case 14: _t->prepareUpdate((*reinterpret_cast< const SourceT(*)>(_a[1]))); break;
        case 15: { SeverityWeightInfoT _r = _t->updateNodeStates((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< SeverityWeightInfoT*>(_a[0]) = _r; }  break;
        case 16: { bool _r = _t->allocSourceHandler((*reinterpret_cast< SourceT(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 17: _t->handleSourceSettingsChanged((*reinterpret_cast< QList<qint8>(*)>(_a[1]))); break;
        case 18: _t->handleErrorOccurred((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 19: _t->initialize((*reinterpret_cast< Preferences*(*)>(_a[1]))); break;
        case 20: { CoreDataT* _r = _t->cdata();
            if (_a[0]) *reinterpret_cast< CoreDataT**>(_a[0]) = _r; }  break;
        case 21: { qint32 _r = _t->userRole();
            if (_a[0]) *reinterpret_cast< qint32*>(_a[0]) = _r; }  break;
        case 22: { bool _r = _t->showOnlyTroubles();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 23: _t->setShowOnlyTroubles((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 24: { SourceListT _r = _t->sources();
            if (_a[0]) *reinterpret_cast< SourceListT*>(_a[0]) = _r; }  break;
        case 25: { int _r = _t->firstSrcIndex();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DashboardBase::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DashboardBase::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DashboardBase,
      qt_meta_data_DashboardBase, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DashboardBase::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DashboardBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DashboardBase::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DashboardBase))
        return static_cast<void*>(const_cast< DashboardBase*>(this));
    return QObject::qt_metacast(_clname);
}

int DashboardBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void DashboardBase::sortEventConsole()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void DashboardBase::updateStatusBar(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DashboardBase::settingsLoaded()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void DashboardBase::updateSourceUrl()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void DashboardBase::timerIntervalChanged(qint32 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DashboardBase::errorOccurred(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void DashboardBase::dashboardLinkSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}

// SIGNAL 7
void DashboardBase::updateInprogress()
{
    QMetaObject::activate(this, &staticMetaObject, 7, 0);
}

// SIGNAL 8
void DashboardBase::updateFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}
QT_END_MOC_NAMESPACE
