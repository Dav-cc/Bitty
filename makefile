CC = gcc

CFLAGS = -g -Wall -Wextra -Werror
CPPFLAGS = -Isrc
LDLIBS = -lcrypto

TARGET = bitty
TEST_TARGET = test_runner
BUILD_DIR = build

SRC = \
	src/bencode.c \
	src/buffer.c \
	src/log.c    \
	src/main.c

TEST_SRC = \
	test/unit/Bencode-parser.c

OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))
TEST_OBJ = $(patsubst test/%.c,$(BUILD_DIR)/test/%.o,$(TEST_SRC))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test/%.o: test/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TEST_TARGET): $(TEST_OBJ) $(OBJ)
	$(CC) $^ -o $@ $(LDLIBS) -lcriterion

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

test: $(BUILD_DIR)/$(TEST_TARGET)
	./$(BUILD_DIR)/$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run test clean
