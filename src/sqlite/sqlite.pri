HEADERS += sqlite/sqlcachedresult.h \
           sqlite/sqlitedriver.h \
           sqlite/sqliteextension.h

SOURCES += sqlite/sqlcachedresult.cpp \
           sqlite/sqlitedriver.cpp \
           sqlite/sqliteextension.cpp

!system-sqlite {
    HEADERS += sqlite/sqlite3.h
    SOURCES += sqlite/sqlite3.c
}
