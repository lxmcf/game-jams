# ----------------------------------------------------------------------------------
#  CONFIG
# ----------------------------------------------------------------------------------
EXE ?= index
BUILD ?= DEBUG

CC := emcc
RUNNER := emrun
COMPILER_FLAGS := -Os -Wall -I/usr/local/include -s USE_GLFW=3 -s ASYNCIFY --preload-file ../assets --shell-file shell.html
SRC_FILES := $(wildcard ../src/*.c) $(wildcard ../src/*/*.c) libraylib.a

# ----------------------------------------------------------------------------------
#  CONFIG SETUP
# ----------------------------------------------------------------------------------
ifeq ($(BUILD), DEBUG)
	LINKER_FLAGS += -g
endif

.PHONY: build run clean release

build:
	$(CC) -o $(EXE).html $(SRC_FILES) $(COMPILER_FLAGS)

run:
	$(RUNNER) $(EXE).html

clean:
	rm $(wildcard $(EXE).*)

release:
	zip -r $(EXE).zip $(wildcard $(EXE).*)
