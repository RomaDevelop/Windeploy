QT       += core gui widgets network

CONFIG += c++17

SOURCES += \
    DeployKit.cpp \
    main.cpp \
    windeploy.cpp

HEADERS += \
    DeployKit.h \
    windeploy.h

FORMS += \
    windeploy.ui

INCLUDEPATH += \
    ../include

DEPENDPATH += \
	../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
