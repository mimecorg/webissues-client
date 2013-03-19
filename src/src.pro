include( ../config.pri )

TEMPLATE = app
TARGET   = webissues

CONFIG  += qt
QT      += network xml sql webkit

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
PRECOMPILED_SOURCE = precompiled.cpp

TRANSLATIONS += \
           ../translations/webissues_untranslated.ts \
           ../translations/webissues_de.ts \
           ../translations/webissues_es.ts \
           ../translations/webissues_fr.ts \
           ../translations/webissues_nl.ts \
           ../translations/webissues_pl.ts \
           ../translations/webissues_pt_BR.ts

win32 {
    RC_FILE = webissues.rc
    LIBS += -lshell32 -lcrypt32
}

mac {
    ICON = webissues.icns
}

mac:ppc {
    LIBS *= -lgcc_eh
}

win32-msvc* {
    QMAKE_CXXFLAGS += -Fd\$(IntDir)
    CONFIG -= flat
}

!win32 | build_pass {
    CONFIG( debug, debug|release ) {
        OBJECTS_DIR = ../tmp/obj/debug
        MOC_DIR = ../tmp/moc/debug
        RCC_DIR = ../tmp/rcc/debug
        DESTDIR = ../debug
    } else {
        OBJECTS_DIR = ../tmp/obj/release
        MOC_DIR = ../tmp/moc/release
        RCC_DIR = ../tmp/rcc/release
        DESTDIR = ../release
    }
    UI_DIR = ../tmp/ui
}

target.path = $${DESTINATION}$$PREFIX/bin
!mac:INSTALLS += target

unix:!mac {
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
