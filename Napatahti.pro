QT += core gui
QT += sql
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++latest

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Appgui/AboutDialog.cpp \
    Appgui/AppTrigger.cpp \
    Appgui/AspDialog.cpp \
    Appgui/AspPageDialog.cpp \
    Appgui/AspPatchDialog.cpp \
    Appgui/AspTableDialog.cpp \
    Appgui/Canvas.cpp \
    Appgui/CanvasBase.cpp \
    Appgui/CatalogDialog.cpp \
    Appgui/ConfigDialog.cpp \
    Appgui/MainWindow.cpp \
    Appgui/PlanetDialog.cpp \
    Appgui/StarOrbDialog.cpp \
    Appgui/TCounterDialog.cpp \
    Kernel/AshaTest.cpp \
    Kernel/AspPage.cpp \
    Kernel/AspTable.cpp \
    Kernel/AstroBase.cpp \
    Kernel/CosmicTest.cpp \
    Kernel/Kernel.cpp \
    Kernel/Person.cpp \
    Appgui/SharedGui.cpp \
    Appgui/AtlasDialog.cpp \
    Appgui/CityDialog.cpp \
    Appgui/DataBaseDialog.cpp \
    Appgui/LineEditDialog.cpp \
    Appgui/PersonDialog.cpp \
    Kernel/PrimeTest.cpp \
    shared.cpp \
    main.cpp \
    static.cpp

HEADERS += \
    Appgui/AboutDialog.h \
    Appgui/AppTrigger.h \
    Appgui/AspDialog.h \
    Appgui/AspPageDialog.h \
    Appgui/AspPatchDialog.h \
    Appgui/AspTableDialog.h \
    Appgui/Canvas.h \
    Appgui/CanvasBase.h \
    Appgui/CatalogDialog.h \
    Appgui/ConfigDialog.h \
    Appgui/MainWindow.h \
    Appgui/PlanetDialog.h \
    Appgui/StarOrbDialog.h \
    Appgui/TCounterDialog.h \
    Kernel/AshaTest.h \
    Kernel/AspPage.h \
    Kernel/AspTable.h \
    Kernel/AstroBase.h \
    Kernel/CosmicTest.h \
    Kernel/Kernel.h \
    Kernel/Person.h \
    Appgui/AtlasDialog.h \
    Appgui/CityDialog.h \
    Appgui/DataBaseDialog.h \
    Appgui/LineEditDialog.h \
    Appgui/PersonDialog.h \
    Appgui/SharedGui.h \
    Kernel/PrimeTest.h \
    Kernel/RefBook.h \
    mask.h \
    shared.h \
    static.h

FORMS += \
    Appgui/AboutDialog.ui \
    Appgui/AspDialog.ui \
    Appgui/AspPageDialog.ui \
    Appgui/AspPatchDialog.ui \
    Appgui/AspTableDialog.ui \
    Appgui/AtlasDialog.ui \
    Appgui/CatalogDialog.ui \
    Appgui/CityDialog.ui \
    Appgui/ConfigDialog.ui \
    Appgui/DataBaseDialog.ui \
    Appgui/LineEditDialog.ui \
    Appgui/MainWindow.ui \
    Appgui/PersonDialog.ui \
    Appgui/PlanetDialog.ui \
    Appgui/StarOrbDialog.ui \
    Appgui/TCounterDialog.ui

TRANSLATIONS += \
    locale/Napatahti_ru_RU.ts

RESOURCES += \
    resource.qrc

win32:RC_ICONS = $$PWD/resource/app.ico
win32:{
    VERSION = 1.0
    QMAKE_TARGET_PRODUCT = Napatahti
    QMAKE_TARGET_DESCRIPTION = Astrological processor for amateurs
    QMAKE_TARGET_COPYRIGHT = Tulemetsan
}

LIBS += "swelib/swedll64.dll"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
