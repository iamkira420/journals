QT += core gui widgets

CONFIG += c++17

TARGET = JournalDB
TEMPLATE = app

# Enable warnings
CONFIG += warn_on

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    articlemodel.cpp \
    articlefilterproxy.cpp

HEADERS += \
    mainwindow.h \
    articlemodel.h \
    articlefilterproxy.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target