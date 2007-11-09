
TEMPLATE = app
QT -= gui

PRECOMPILED_DIR = .pch/
OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .uic

QMAKE_CFLAGS = -g -O0 -pedantic $(CUSTOM_OPT_FLAGS)
QMAKE_CXXFLAGS = -g -O0 -pedantic $(CUSTOM_OPT_FLAGS)

SOURCES = main.cpp
