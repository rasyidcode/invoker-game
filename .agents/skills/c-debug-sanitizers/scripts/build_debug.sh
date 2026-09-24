#!/usr/bin/env bash
# Helper script to build invoker_game with AddressSanitizer and debug symbols
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
cd "$REPO_ROOT"

echo "Building invoker_game with AddressSanitizer (ASan) and UBSan..."

gcc -Wall -Wextra -std=c99 -g -O0 \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -Iinclude -I/usr/local/include \
    src/*.c \
    -o invoker_game_debug \
    -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
    -fsanitize=address,undefined

echo "Debug build successful: ./invoker_game_debug"
echo "Run it directly to catch buffer overflows, use-after-free, and memory corruption:"
echo "  ./invoker_game_debug"
