/****************************************************************************
** Meta object code from reading C++ file 'SvNavigator.hpp'
**
** Created: Wed Jun 19 20:58:54 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/client/SvNavigator.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SvNavigator.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SvNavigator[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      36,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      55,   12,   12,   12, 0x0a,
      74,   12,   70,   12, 0x0a,
      93,   12,   70,   12, 0x0a,
     108,   12,   12,   12, 0x0a,
     139,  133,   12,   12, 0x0a,
     184,  161,   12,   12, 0x0a,
     224,  216,   12,   12, 0x0a,
     251,   12,   12,   12, 0x2a,
     271,   12,   12,   12, 0x0a,
     294,   12,   12,   12, 0x0a,
     324,   12,   12,   12, 0x0a,
     338,   12,   12,   12, 0x0a,
     354,   12,   12,   12, 0x0a,
     366,   12,   12,   12, 0x0a,
     402,   12,   12,   12, 0x0a,
     431,   12,   12,   12, 0x0a,
     470,   12,   12,   12, 0x0a,
     498,   12,   12,   12, 0x0a,
     525,  516,   12,   12, 0x0a,
     548,  516,   12,   12, 0x0a,
     572,  516,   12,   12, 0x0a,
     606,  600,   12,   12, 0x0a,
     638,  600,   12,   12, 0x0a,
     675,  670,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SvNavigator[] = {
    "SvNavigator\0\0hasToBeUpdate(QString)\0"
    "sortEventConsole()\0startMonitor()\0int\0"
    "runNagiosMonitor()\0runLsMonitor()\0"
    "prepareDashboardUpdate()\0_node\0"
    "updateBpNode(QString)\0_nodeId,_expand,_level\0"
    "expandNode(QString,bool,qint32)\0_nodeId\0"
    "centerGraphOnNode(QString)\0"
    "centerGraphOnNode()\0filterNodeRelatedMsg()\0"
    "filterNodeRelatedMsg(QString)\0"
    "acknowledge()\0tabChanged(int)\0hideChart()\0"
    "centerGraphOnNode(QTreeWidgetItem*)\0"
    "handleChangePasswordAction()\0"
    "handleChangeMonitoringSettingsAction()\0"
    "handleShowOnlineResources()\0"
    "handleShowAbout()\0_toggled\0"
    "toggleFullScreen(bool)\0toggleTroubleView(bool)\0"
    "toggleIncreaseMsgFont(bool)\0reply\0"
    "processZbxReply(QNetworkReply*)\0"
    "processZnsReply(QNetworkReply*)\0code\0"
    "processRpcError(QNetworkReply::NetworkError)\0"
};

const QMetaObject SvNavigator::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SvNavigator,
      qt_meta_data_SvNavigator, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SvNavigator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SvNavigator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SvNavigator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SvNavigator))
        return static_cast<void*>(const_cast< SvNavigator*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int SvNavigator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: hasToBeUpdate((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: sortEventConsole(); break;
        case 2: startMonitor(); break;
        case 3: { int _r = runNagiosMonitor();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = runLsMonitor();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: prepareDashboardUpdate(); break;
        case 6: updateBpNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: expandNode((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2])),(*reinterpret_cast< const qint32(*)>(_a[3]))); break;
        case 8: centerGraphOnNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: centerGraphOnNode(); break;
        case 10: filterNodeRelatedMsg(); break;
        case 11: filterNodeRelatedMsg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: acknowledge(); break;
        case 13: tabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: hideChart(); break;
        case 15: centerGraphOnNode((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 16: handleChangePasswordAction(); break;
        case 17: handleChangeMonitoringSettingsAction(); break;
        case 18: handleShowOnlineResources(); break;
        case 19: handleShowAbout(); break;
        case 20: toggleFullScreen((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 21: toggleTroubleView((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 22: toggleIncreaseMsgFont((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: processZbxReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 24: processZnsReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 25: processRpcError((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void SvNavigator::hasToBeUpdate(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SvNavigator::sortEventConsole()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
