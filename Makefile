HEADERS = gv.h

TESTS_SRC =               \
	tests/test.c          \
	tests/simple_http.c   \
	tests/simple_hex.c    \
	tests/fake_malloc.c   \
	tests/fake_memset.c   \
	tests/thread_pool.c   \
	tests/pre_main.c      \

OUTPUT_DIR=./build

CC = gcc
CFLAGS = -ldl -lpthread -pthread -Wall -I . -std=c11

TESTS2 = $(TESTS_SRC:tests/%=%)
TESTS1 = $(TESTS2:.c=)
TESTS = $(addprefix $(OUTPUT_DIR)/,$(TESTS1))

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(TESTS): $(TESTS_SRC) $(HEADERS)
	$(CC) -o $@ tests/$(basename $(notdir $@)).c $(CFLAGS)

build_tests: $(OUTPUT_DIR) $(TESTS)

clean:
	-rm -f $(TESTS)
	-rm -r $(OUTPUT_DIR)

.PNONY: clean
default: build_tests
all: default