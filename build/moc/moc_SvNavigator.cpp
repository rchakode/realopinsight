/****************************************************************************
** Meta object code from reading C++ file 'SvNavigator.hpp'
**
** Created: Fri Mar 1 20:21:01 2013
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
      22,   14, // methods
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
      93,   12,   12,   12, 0x0a,
     124,  118,   12,   12, 0x0a,
     169,  146,   12,   12, 0x0a,
     209,  201,   12,   12, 0x0a,
     236,   12,   12,   12, 0x2a,
     256,   12,   12,   12, 0x0a,
     279,   12,   12,   12, 0x0a,
     309,   12,   12,   12, 0x0a,
     323,   12,   12,   12, 0x0a,
     339,   12,   12,   12, 0x0a,
     351,   12,   12,   12, 0x0a,
     387,   12,   12,   12, 0x0a,
     416,   12,   12,   12, 0x0a,
     455,   12,   12,   12, 0x0a,
     483,   12,   12,   12, 0x0a,
     507,  501,   12,   12, 0x0a,
     539,  501,   12,   12, 0x0a,
     576,  571,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SvNavigator[] = {
    "SvNavigator\0\0hasToBeUpdate(QString)\0"
    "sortEventConsole()\0startMonitor()\0int\0"
    "runNagiosMonitor()\0prepareDashboardUpdate()\0"
    "_node\0updateBpNode(QString)\0"
    "_nodeId,_expand,_level\0"
    "expandNode(QString,bool,qint32)\0_nodeId\0"
    "centerGraphOnNode(QString)\0"
    "centerGraphOnNode()\0filterNodeRelatedMsg()\0"
    "filterNodeRelatedMsg(QString)\0"
    "acknowledge()\0tabChanged(int)\0hideChart()\0"
    "centerGraphOnNode(QTreeWidgetItem*)\0"
    "handleChangePasswordAction()\0"
    "handleChangeMonitoringSettingsAction()\0"
    "handleShowOnlineResources()\0"
    "handleShowAbout()\0reply\0"
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
        case 4: prepareDashboardUpdate(); break;
        case 5: updateBpNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: expandNode((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2])),(*reinterpret_cast< const qint32(*)>(_a[3]))); break;
        case 7: centerGraphOnNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: centerGraphOnNode(); break;
        case 9: filterNodeRelatedMsg(); break;
        case 10: filterNodeRelatedMsg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: acknowledge(); break;
        case 12: tabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: hideChart(); break;
        case 14: centerGraphOnNode((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 15: handleChangePasswordAction(); break;
        case 16: handleChangeMonitoringSettingsAction(); break;
        case 17: handleShowOnlineResources(); break;
        case 18: handleShowAbout(); break;
        case 19: processZbxReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 20: processZnsReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 21: processRpcError((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 22;
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
