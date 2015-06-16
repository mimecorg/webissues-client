HEADERS += $$PWD/sqlcachedresult.h \
           $$PWD/sqlitedriver.h \
           $$PWD/sqliteextension.h

SOURCES += $$PWD/sqlcachedresult.cpp \
           $$PWD/sqlitedriver.cpp \
           $$PWD/sqliteextension.cpp

system-sqlite {
    DEFINES += HAVE_SYSTEM_SQLITE
    LIBS += -lsqlite3
} else {
    DEFINES += SQLITE_OMIT_LOAD_EXTENSION SQLITE_OMIT_COMPLETE
    HEADERS += $$PWD/sqlite3.h
    SOURCES += $$PWD/sqlite3.c
}
