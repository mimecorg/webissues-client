HEADERS += dialogs/alertdialogs.h \
           dialogs/checkmessagebox.h \
           dialogs/commanddialog.h \
           dialogs/connectioninfodialog.h \
           dialogs/finditemdialog.h \
           dialogs/informationdialog.h \
           dialogs/issuedialogs.h \
           dialogs/logindialog.h \
           dialogs/managealertsdialog.h \
           dialogs/membersdialog.h \
           dialogs/metadatadialog.h \
           dialogs/preferencesdialog.h \
           dialogs/projectdialogs.h \
           dialogs/reportdialog.h \
           dialogs/settingsdialog.h \
           dialogs/statedialogs.h \
           dialogs/typedialogs.h \
           dialogs/userdialogs.h \
           dialogs/viewdialogs.h \
           dialogs/viewsettingsdialog.h

SOURCES += dialogs/alertdialogs.cpp \
           dialogs/checkmessagebox.cpp \
           dialogs/commanddialog.cpp \
           dialogs/connectioninfodialog.cpp \
           dialogs/finditemdialog.cpp \
           dialogs/informationdialog.cpp \
           dialogs/issuedialogs.cpp \
           dialogs/logindialog.cpp \
           dialogs/managealertsdialog.cpp \
           dialogs/metadatadialog.cpp \
           dialogs/membersdialog.cpp \
           dialogs/preferencesdialog.cpp \
           dialogs/projectdialogs.cpp \
           dialogs/reportdialog.cpp \
           dialogs/settingsdialog.cpp \
           dialogs/statedialogs.cpp \
           dialogs/typedialogs.cpp \
           dialogs/userdialogs.cpp \
           dialogs/viewdialogs.cpp \
           dialogs/viewsettingsdialog.cpp

contains( QT_CONFIG, openssl ) | contains( QT_CONFIG, openssl-linked ) {
    HEADERS += dialogs/ssldialogs.h
    SOURCES += dialogs/ssldialogs.cpp
}
