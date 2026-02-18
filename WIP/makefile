CC := odin
BUILD ?= DEBUG

C_FLAGS := -vet -min-link-libs -strict-style -disallow-do -linker:mold
OPT := none

EXE_EXT :=

ifeq ($(BUILD),DEBUG)
	C_FLAGS += -debug -microarch:native
else ifeq ($(BUILD), RELEASE)
	OPT = speed
endif

ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe

	C_FLAGS += -subsystem:windows
endif

.PHONY: build run clean

build:
	mkdir -p build
	odin build src/ -out:build/game$(EXE_EXT) $(C_FLAGS) -o:$(OPT)

run: build
	build/game$(EXT_EXT)

run_mango: build
	mangohud build/game$(EXT_EXT)

clean:
ifeq ($(OS), Windows_NT)
	rmdir /s build/
else
	rm -rf build/
endif
