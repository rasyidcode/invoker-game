# Compiler & Flags for Desktop (Linux x86_64)
CC ?= gcc

RAYLIB_PATH ?= /usr/local/include

CFLAGS = -Wall -Wextra -std=c99 -Iinclude -I$(RAYLIB_PATH) -MMD -MP
DEBUG_CFLAGS = -Wall -Wextra -std=c99 -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude -I$(RAYLIB_PATH) -MMD -MP

LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
DEBUG_LDFLAGS = $(LDFLAGS) -fsanitize=address,undefined

# Directories
BUILD_DIR = build
DESKTOP_DIR = $(BUILD_DIR)/desktop
DESKTOP_OBJ_DIR = $(DESKTOP_DIR)/obj
DESKTOP_DEBUG_OBJ_DIR = $(DESKTOP_DIR)/obj_debug

SRC = $(wildcard src/*.c)
#OBJ = $(SRC:.c=.o)
OBJ = $(patsubst src/%.c, $(DESKTOP_OBJ_DIR)/%.o, $(SRC))
DEP = $(OBJ:.o=.d)

DEBUG_OBJ = $(patsubst src/%.c, $(DESKTOP_DEBUG_OBJ_DIR)/%.o, $(SRC))
DEBUG_DEP = $(DEBUG_OBJ:.o=.d)

TARGET = $(DESKTOP_DIR)/invoker_game
DEBUG_TARGET = $(DESKTOP_DIR)/invoker_game_debug

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

# Desktop Release Target
all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(DESKTOP_DIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(DESKTOP_OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(DESKTOP_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Desktop Debug Target (with ASan & UBSan)
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJ)
	@mkdir -p $(DESKTOP_DIR)
	$(CC) $(DEBUG_OBJ) -o $@ $(DEBUG_LDFLAGS)

$(DESKTOP_DEBUG_OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(DESKTOP_DEBUG_OBJ_DIR)
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

-include $(DEP)
-include $(DEBUG_DEP)

run: $(TARGET)
	./$(TARGET)

run-debug: $(DEBUG_TARGET)
	./$(DEBUG_TARGET)

# Web Target
web: $(WEB_TARGET)

$(WEB_TARGET): $(SRC) $(WEB_SHELL)
	@mkdir -p $(WEB_OUT_DIR)
	$(EMCC) -o $(WEB_TARGET) $(SRC) $(EMCC_FLAGS)
	@echo "Web build ready in $(WEB_OUT_DIR)/"

run-web: web
	@echo "Starting local web server at http://localhost:8080..."
	python3 -m http.server 8080 --directory $(WEB_OUT_DIR)

# Clean Targets
clean: clean-desktop

clean-desktop:
	# rm -f src/*.o src/*.d $(TARGET)
	rm -rf $(DESKTOP_DIR)

clean-web:
	rm -rf $(WEB_OUT_DIR)

clean-all:
	rm -rf $(BUILD_DIR)

.PHONY: all run debug run-debug web run-web clean clean-desktop clean-web clean-all
