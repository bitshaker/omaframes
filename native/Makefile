CXX ?= g++
CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++2b -Wall -Wextra -Wpedantic

ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

SOURCES = src/main.cpp src/VineDecoration.cpp src/VinePassElement.cpp
TARGET = omavines-native.so
HYPRLAND_CFLAGS = $(shell pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) src/VineDecoration.hpp src/VinePassElement.hpp src/globals.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS) $(HYPRLAND_CFLAGS) $(SOURCES) -o $@

clean:
	$(RM) $(TARGET)
