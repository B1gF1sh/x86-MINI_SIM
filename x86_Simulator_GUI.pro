QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    codeeditor.cpp \
    main.cpp \
    mainwindow.cpp \
    /home/roo0t/Desktop/_Assembler_SIM/x86-Simulator/src/cpu.cpp \
    /home/roo0t/Desktop/_Assembler_SIM/x86-Simulator/src/parser.cpp \
    stardialog.cpp \
    userdialog.cpp

HEADERS += \
    codeeditor.h \
    mainwindow.h \
    /home/roo0t/Desktop/_Assembler_SIM/x86-Simulator/src/cpu.h \
    /home/roo0t/Desktop/_Assembler_SIM/x86-Simulator/src/parser.h \
    stardialog.h \
    userdialog.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += /home/roo0t/Desktop/_Assembler_SIM/x86-Simulator/src


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
