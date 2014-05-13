#configuration
# If we install this into an in-builddir deployment of Qt, it would confuse
# the next build of Qt. So pretend we're not there in this case.
# This file is loaded by qt_config.prf, before .qmake.cache has been loaded.
# Consequently, we have to do some stunts to get values out of the cache.
if(!exists($$_QMAKE_CACHE_)| \
   !contains($$list($$fromfile($$_QMAKE_CACHE_, CONFIG)), QTDIR_build)): \
    QT_CONFIG += phonon
