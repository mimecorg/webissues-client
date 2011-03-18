include( config.pri )

TEMPLATE = subdirs
SUBDIRS  = src

# NOTE: if you change the installation paths, please update application.cpp accordingly

documentation.files = ChangeLog COPYING README
win32 {
    documentation.path = $$PREFIX
} else:mac {
    documentation.path = $${DESTINATION}$$PREFIX
} else {
    documentation.path = $${DESTINATION}$$PREFIX/share/doc/webissues
}
INSTALLS += documentation

translations.files = translations/*.qm translations/locale.ini
win32 {
    translations.path = $$PREFIX/translations
} else:mac {
    translations.path = $${DESTINATION}$$PREFIX/translations
} else {
    translations.path = $${DESTINATION}$$PREFIX/share/webissues/translations
}
INSTALLS += translations
