HEADERS = ../../gv.h code_gen.h
OUTPUT_DIR=./build
CC = gcc
CFLAGS = -ldl -lpthread -pthread -Wall -std=c11 -D _BSD_SOURCE -lm \
    -I .                                        \
    -I ../..                                    \
    -I ../../3rdparty/stb                       \
    -I ../../3rdparty/glfw/include              \
    -I ../../3rdparty/nuklear                   \
    -I ../../3rdparty                           \
    -D GV_CODE_GEN_IMPLEMENATION                \
    -D GV_CODE_GEN_MAIN                         \
    -D STB_DEFINE                               \
    -D STB_C_LEXER_IMPLEMENTATION               \
    -D GV_IMPLEMENTATION

$(OUTPUT_DIR)/code_gen: $(HEADERS)
	mkdir -p $(OUTPUT_DIR)
	$(CC) -o $@ -x c code_gen.h $(CFLAGS) 

build: $(OUTPUT_DIR)/code_gen

clean:
	-rm -f $(OUTPUT_DIR)/code_gen
	-rm -r $(OUTPUT_DIR)

.PNONY: clean
default: build
all: default