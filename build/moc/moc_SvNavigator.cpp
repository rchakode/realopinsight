/****************************************************************************
** Meta object code from reading C++ file 'SvNavigator.hpp'
**
** Created: Thu Aug 15 17:46:46 2013
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
      33,   14, // methods
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
      72,   68,   12,   12, 0x0a,
      99,   93,   12,   12, 0x0a,
     120,   68,   12,   12, 0x0a,
     145,   93,   12,   12, 0x0a,
     170,   68,   12,   12, 0x0a,
     199,   12,   12,   12, 0x0a,
     215,   68,   12,   12, 0x0a,
     244,  238,   12,   12, 0x0a,
     289,  266,   12,   12, 0x0a,
     329,  321,   12,   12, 0x0a,
     356,   12,   12,   12, 0x2a,
     376,   12,   12,   12, 0x0a,
     399,   12,   12,   12, 0x0a,
     429,   12,   12,   12, 0x0a,
     443,   12,   12,   12, 0x0a,
     459,   12,   12,   12, 0x0a,
     471,   12,   12,   12, 0x0a,
     507,   12,   12,   12, 0x0a,
     536,   12,   12,   12, 0x0a,
     575,   12,   12,   12, 0x0a,
     603,   12,   12,   12, 0x0a,
     630,  621,   12,   12, 0x0a,
     653,  621,   12,   12, 0x0a,
     677,  621,   12,   12, 0x0a,
     715,  705,   12,   12, 0x0a,
     756,  705,   12,   12, 0x0a,
     806,  797,   12,   12, 0x0a,
     864,   68,  859,   12, 0x0a,
     897,  893,   12,   12, 0x0a,
     945,  939,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SvNavigator[] = {
    "SvNavigator\0\0hasToBeUpdate(QString)\0"
    "sortEventConsole()\0runMonitor()\0src\0"
    "runMonitor(SourceT&)\0srcId\0"
    "runNagiosUpdate(int)\0runNagiosUpdate(SourceT)\0"
    "runLivestatusUpdate(int)\0"
    "runLivestatusUpdate(SourceT)\0"
    "resetStatData()\0prepareUpdate(SourceT)\0"
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
    "handleShowAbout()\0_toggled\0"
    "toggleFullScreen(bool)\0toggleTroubleView(bool)\0"
    "toggleIncreaseMsgFont(bool)\0reply,src\0"
    "processZbxReply(QNetworkReply*,SourceT&)\0"
    "processZnsReply(QNetworkReply*,SourceT&)\0"
    "code,src\0processRpcError(QNetworkReply::NetworkError,SourceT)\0"
    "bool\0allocSourceHandler(SourceT&)\0ids\0"
    "handleSourceSettingsChanged(QList<qint8>)\0"
    "index\0handleSourceBxItemChanged(int)\0"
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
        case 2: runMonitor(); break;
        case 3: runMonitor((*reinterpret_cast< SourceT(*)>(_a[1]))); break;
        case 4: runNagiosUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: runNagiosUpdate((*reinterpret_cast< const SourceT(*)>(_a[1]))); break;
        case 6: runLivestatusUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: runLivestatusUpdate((*reinterpret_cast< const SourceT(*)>(_a[1]))); break;
        case 8: resetStatData(); break;
        case 9: prepareUpdate((*reinterpret_cast< const SourceT(*)>(_a[1]))); break;
        case 10: updateBpNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: expandNode((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2])),(*reinterpret_cast< const qint32(*)>(_a[3]))); break;
        case 12: centerGraphOnNode((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: centerGraphOnNode(); break;
        case 14: filterNodeRelatedMsg(); break;
        case 15: filterNodeRelatedMsg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: acknowledge(); break;
        case 17: tabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: hideChart(); break;
        case 19: centerGraphOnNode((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 20: handleChangePasswordAction(); break;
        case 21: handleChangeMonitoringSettingsAction(); break;
        case 22: handleShowOnlineResources(); break;
        case 23: handleShowAbout(); break;
        case 24: toggleFullScreen((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 25: toggleTroubleView((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 26: toggleIncreaseMsgFont((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 27: processZbxReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1])),(*reinterpret_cast< SourceT(*)>(_a[2]))); break;
        case 28: processZnsReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1])),(*reinterpret_cast< SourceT(*)>(_a[2]))); break;
        case 29: processRpcError((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1])),(*reinterpret_cast< const SourceT(*)>(_a[2]))); break;
        case 30: { bool _r = allocSourceHandler((*reinterpret_cast< SourceT(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 31: handleSourceSettingsChanged((*reinterpret_cast< QList<qint8>(*)>(_a[1]))); break;
        case 32: handleSourceBxItemChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 33;
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
