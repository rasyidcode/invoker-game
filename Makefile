# Compiler & Flags for Desktop (Linux x86_64)
CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = invoker_game

# Emscripten toolchain for WebAssembly (HTML5)
EMSDK_PATH ?= /home/rcd/Applications/emsdk-6.0.10
EMCC ?= $(shell which emcc 2>/dev/null || echo $(EMSDK_PATH)/upstream/emscripten/emcc)
RAYLIB_WEB_PATH ?= /home/rcd/My-Work/raylib

WEB_OUT_DIR = build/web
WEB_TARGET = $(WEB_OUT_DIR)/index.html
WEB_SHELL = src/shell.html

EMCC_FLAGS = -Os -Wall -Wextra -std=c99 -Iinclude -I$(RAYLIB_WEB_PATH)/src \
             $(RAYLIB_WEB_PATH)/src/libraylib.a \
             -s USE_GLFW=3 -DPLATFORM_WEB \
             --preload-file assets \
             --shell-file $(WEB_SHELL) \
             -s TOTAL_MEMORY=67108864 \
             -s FORCE_FILESYSTEM=1

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

web: $(WEB_TARGET)

$(WEB_TARGET): $(SRC) $(WEB_SHELL)
	@mkdir -p $(WEB_OUT_DIR)
	$(EMCC) -o $(WEB_TARGET) $(SRC) $(EMCC_FLAGS)
	@echo "Web build ready in $(WEB_OUT_DIR)/"

run-web: web
	@echo "Starting local web server at http://localhost:8080..."
	python3 -m http.server 8080 --directory $(WEB_OUT_DIR)

clean:
	rm -f src/*.o $(TARGET)
	rm -rf $(WEB_OUT_DIR)

clean-web:
	rm -rf $(WEB_OUT_DIR)

.PHONY: all run clean web run-web clean-web
