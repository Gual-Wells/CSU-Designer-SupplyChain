QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += D:\EIGEN\eigen_3_4_0\eigen_3_4_0

QT += charts



# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chartwidget.cpp \
    choosedialog.cpp \
    client.cpp \
    edgedata.cpp \
    edgeitem.cpp \
    edgepropertydialog.cpp \
    fw.cpp \
    global.cpp \
    graphscene.cpp \
    graphview.cpp \
    main.cpp \
    mainwindow.cpp \
    nodedata.cpp \
    nodeitem.cpp \
    nodepropertydialog.cpp \
    order.cpp \
    path.cpp \
    population.cpp \
    population_and_order.cpp \
    result.cpp \
    store.cpp

HEADERS += \
    chartwidget.h \
    choosedialog.h \
    client.h \
    edgedata.h \
    edgeitem.h \
    edgepropertydialog.h \
    fw.h \
    graphscene.h \
    graphview.h \
    mainwindow.h \
    nodedata.h \
    nodeitem.h \
    nodepropertydialog.h \
    order.h \
    path.h \
    population.h \
    population_and_order.h \
    result.h \
    store.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../../DESKTOP/client.png \
    ../../../DESKTOP/storage.png \
    client.png \
    storage.png

RESOURCES += \
    PNG.qrc
