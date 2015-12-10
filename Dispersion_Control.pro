#-------------------------------------------------
#
# Project created by QtCreator 2015-11-19T16:32:12
#
#-------------------------------------------------

QT       += core gui widgets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dispersion_Control
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        serial_controller.cpp \
        add_functions.cpp \
        ../Test_Audio_processing/audioin.cpp \
    monochrom.cpp \
    stepperm.cpp \
    audio_scanner.cpp \
    data_cleaner.cpp

HEADERS  += mainwindow.h\
            serial_controller.h \
            add_functions.h \
            ../Test_Audio_processing/audioin.h \
    monochrom.h \
    stepperm.h \
    audio_scanner.h \
    data_cleaner.h

QMAKE_CXXFLAGS += -std=c++11 -std=gnu++0x -O3 -fopenmp

INCLUDEPATH += /usr/local/qwt-6.1.1/include
#Has to be changed back to -L/usr/lib/ afterwards
LIBS += -L/usr/local/qwt-6.1.1/lib/ -lqwt -lssh -lm -lasound -L/home/roland/qt-workspace/Dispersion_Control -lportaudio

QMAKE_CXXFLAGS += -std=c++11 -std=gnu++0x -O3 -fopenmp

FORMS    += mainwindow.ui
