include( config.pri )

TEMPLATE = subdirs
SUBDIRS  = src

# NOTE: if you change the installation paths, please update application.cpp accordingly

win32 {
    documentation.files = ChangeLog COPYING README
    documentation.path = $$PREFIX
} else:mac {
    documentation.files = ChangeLog COPYING README
    documentation.path = $${DESTINATION}$$PREFIX
} else {
    documentation.files = README
    documentation.path = $${DESTINATION}$$PREFIX/share/doc/webissues
}
INSTALLS += documentation

manual.files = doc/en doc/pl
win32 {
    manual.path = $$PREFIX/doc
} else:mac {
    manual.path = $${DESTINATION}$$PREFIX/doc
} else {
    manual.path = $${DESTINATION}$$PREFIX/share/doc/webissues/doc
}
INSTALLS += manual

translations.files = translations/*.qm translations/locale.ini
win32 {
    translations.path = $$PREFIX/translations
} else:mac {
    translations.path = $${DESTINATION}$$PREFIX/translations
} else {
    translations.path = $${DESTINATION}$$PREFIX/share/webissues/translations
}
INSTALLS += translations

QMAKE_DISTCLEAN += config.pri
