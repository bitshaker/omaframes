CXX ?= g++
CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++2b -Wall -Wextra -Wpedantic

ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

SOURCES = src/packs/accents/AccentPack.cpp src/packs/accents/AccentDecoration.cpp src/packs/accents/AccentPassElement.cpp \
	 src/main.cpp src/packs/BuiltInPacks.cpp \
	src/packs/vines/VinePack.cpp src/packs/vines/VineDecoration.cpp src/packs/vines/VinePassElement.cpp \
	src/packs/critter/CritterPack.cpp src/packs/critter/CritterDirector.cpp src/packs/critter/CritterDecoration.cpp src/packs/critter/CritterPassElement.cpp
HEADERS = src/packs/accents/AccentPack.hpp src/packs/accents/AccentDecoration.hpp src/packs/accents/AccentPassElement.hpp \
	 src/globals.hpp src/packs/BuiltInPacks.hpp \
	src/packs/vines/VinePack.hpp src/packs/vines/VineDecoration.hpp src/packs/vines/VinePassElement.hpp \
	src/packs/critter/CritterPack.hpp src/packs/critter/CritterDirector.hpp src/packs/critter/CritterDecoration.hpp src/packs/critter/CritterPassElement.hpp
TARGET = omaframes-native.so
HYPRLAND_CFLAGS = $(shell pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon)
HYPRLAND_LIBS = $(shell pkg-config --libs pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon)

.PHONY: all clean FORCE

all: $(TARGET)

# Library/header updates must invalidate a previously built development host.
.build-environment: FORCE
	@{ pkg-config --modversion hyprland aquamarine hyprutils hyprgraphics hyprcursor hyprlang; \
	    printf '%s\n' '$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS)' '$(HYPRLAND_CFLAGS)' '$(HYPRLAND_LIBS)'; } > $@.tmp
	@cmp -s $@.tmp $@ && rm -f $@.tmp || mv -f $@.tmp $@

FORCE:

$(TARGET): $(SOURCES) $(HEADERS) .build-environment
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS) $(HYPRLAND_CFLAGS) $(SOURCES) $(HYPRLAND_LIBS) -o $@

clean:
	$(RM) $(TARGET) .build-environment .build-environment.tmp
