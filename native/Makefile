CXX ?= g++
CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++2b -Wall -Wextra -Wpedantic

ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

SOURCES = src/main.cpp src/packs/BuiltInPacks.cpp \
	src/packs/vines/VinePack.cpp src/packs/vines/VineDecoration.cpp src/packs/vines/VinePassElement.cpp \
	src/packs/critter/CritterPack.cpp src/packs/critter/CritterDirector.cpp src/packs/critter/CritterDecoration.cpp src/packs/critter/CritterPassElement.cpp
HEADERS = src/globals.hpp src/packs/BuiltInPacks.hpp \
	src/packs/vines/VinePack.hpp src/packs/vines/VineDecoration.hpp src/packs/vines/VinePassElement.hpp \
	src/packs/critter/CritterPack.hpp src/packs/critter/CritterDirector.hpp src/packs/critter/CritterDecoration.hpp src/packs/critter/CritterPassElement.hpp
TARGET = omaframes-native.so
HYPRLAND_CFLAGS = $(shell pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon)
HYPRLAND_LIBS = $(shell pkg-config --libs pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS) $(HYPRLAND_CFLAGS) $(SOURCES) $(HYPRLAND_LIBS) -o $@

clean:
	$(RM) $(TARGET)
