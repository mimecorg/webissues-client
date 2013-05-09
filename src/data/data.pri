HEADERS += data/bookmark.h \
           data/bookmarksstore.h \
           data/credential.h \
           data/credentialsstore.h \
           data/datamanager.h \
           data/entities.h \
           data/entities_p.h \
           data/filecache.h \
           data/issuetypecache.h \
           data/localsettings.h \
           data/query.h \
           data/updateevent.h

SOURCES += data/bookmark.cpp \
           data/bookmarksstore.cpp \
           data/credential.cpp \
           data/credentialsstore.cpp \
           data/datamanager.cpp \
           data/entities.cpp \
           data/filecache.cpp \
           data/issuetypecache.cpp \
           data/localsettings.cpp \
           data/query.cpp \
           data/updateevent.cpp

contains( QT_CONFIG, openssl ) | contains( QT_CONFIG, openssl-linked ) {
    HEADERS += data/certificatesstore.h
    SOURCES += data/certificatesstore.cpp
}
