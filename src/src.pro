include( ../config.pri )

TEMPLATE = app
TARGET   = webissues

CONFIG  += qt
QT      += network xml sql

HEADERS += application.h \
           mainwindow.h

SOURCES += application.cpp \
           main.cpp \
           mainwindow.cpp

RESOURCES += \
           icons/icons.qrc \
           resources/resources.qrc

include( commands/commands.pri )
include( data/data.pri )
include( dialogs/dialogs.pri )
include( models/models.pri )
include( rdb/rdb.pri )
include( sqlite/sqlite.pri )
include( utils/utils.pri )
include( views/views.pri )
include( widgets/widgets.pri )
include( xmlui/xmlui.pri )

INCLUDEPATH += .

contains( QT_CONFIG, openssl ) | contains( QT_CONFIG, openssl-linked ) {
    DEFINES += HAVE_OPENSSL
}

PRECOMPILED_HEADER = precompiled.h

TRANSLATIONS += \
           ../translations/webissues_untranslated.ts \
           ../translations/webissues_fr.ts \
           ../translations/webissues_pl.ts \
           ../translations/webissues_pt_BR.ts

win32 {
    RC_FILE = webissues.rc
    LIBS += -lshell32 -lcrypt32
}

mac {
    ICON = webissues.icns
}

win32-msvc* {
    QMAKE_CXXFLAGS += -Fd\$(IntDir)
    CONFIG -= flat
}

!win32 | build_pass {
    MOC_DIR = ../tmp
    RCC_DIR = ../tmp
    UI_DIR = ../tmp
    CONFIG( debug, debug|release ) {
        OBJECTS_DIR = ../tmp/debug
        DESTDIR = ../debug
    } else {
        OBJECTS_DIR = ../tmp/release
        DESTDIR = ../release
    }
}

target.path = $${DESTINATION}$$PREFIX/bin
INSTALLS += target

unix {
    desktop.files = webissues.desktop
    desktop.path = $${DESTINATION}$$PREFIX/share/applications
    INSTALLS += desktop

    ICON_SIZES = 16 22 32 48
    for( size, ICON_SIZES ) {
        path = $${DESTINATION}$$PREFIX/share/icons/hicolor/$${size}x$${size}/apps
        file = $(INSTALL_ROOT)$$path/webissues.png
        eval( icon-$${size}.path = $$path )
        eval( icon-$${size}.commands = -$(INSTALL_FILE) $$IN_PWD/icons/webissues-$${size}.png $$file )
        eval( icon-$${size}.uninstall = -$(DEL_FILE) $$file )
        INSTALLS += icon-$${size}
    }

    man.files = webissues.1
    man.path = $${DESTINATION}$$PREFIX/share/man/man1
    INSTALLS += man
}
