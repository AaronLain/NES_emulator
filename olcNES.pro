QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        bus.cpp \
        main.cpp \
        olc6502.cpp

TRANSLATIONS += \
    olcNES_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

INCLUDEPATH += /usr/local/include
INCLUDEPATH += /System/Library/Frameworks
LIBS += -L/usr/local/Cellar/libpng



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    bus.h \
    olc6502.h \

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
