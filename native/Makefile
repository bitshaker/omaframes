CXX ?= g++
CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++2b -Wall -Wextra -Wpedantic

ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

SOURCES = src/main.cpp src/packs/vines/VinePack.cpp src/packs/vines/VineDecoration.cpp src/packs/vines/VinePassElement.cpp
HEADERS = src/globals.hpp src/packs/vines/VinePack.hpp src/packs/vines/VineDecoration.hpp src/packs/vines/VinePassElement.hpp
TARGET = omaframes-native.so
HYPRLAND_CFLAGS = $(shell pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS) $(HYPRLAND_CFLAGS) $(SOURCES) -o $@

clean:
	$(RM) $(TARGET)
