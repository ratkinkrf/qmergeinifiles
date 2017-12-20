
TEMPLATE = app
QT -= gui

PRECOMPILED_DIR = .pch/
OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

target.path = /usr/bin/
INSTALLS += target

QMAKE_CFLAGS += -pedantic $(CUSTOM_OPT_FLAGS)
QMAKE_CXXFLAGS += -pedantic $(CUSTOM_OPT_FLAGS)

SOURCES = main.cpp
