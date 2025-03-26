
CC := $(shell which gcc || which clang || which tcc)
CFLAGS := -Wall -Wextra -fPIC -I./src
LDFLAGS := -shared
TEST_LDFLAGS := -Lsrc -lomena
LIB_DIR := src
SHARED_LIB := $(LIB_DIR)/libomena.so
OBJ_DIR := $(LIB_DIR)/build
TEST_DIR := test
TEST_BUILD_DIR := $(TEST_DIR)/build
SRC_FILES := $(wildcard $(LIB_DIR)/*.c)
OBJ_FILES := $(patsubst $(LIB_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
TEST_FILES := $(wildcard $(TEST_DIR)/spec_*.c)
TEST_EXECS := $(patsubst $(TEST_DIR)/%.c, $(TEST_BUILD_DIR)/%, $(TEST_FILES))

all: $(SHARED_LIB)

$(SHARED_LIB): $(OBJ_FILES)
	@echo "Creating shared library..."
	$(CC) $(LDFLAGS) -o $@ src/build/boot.o src/build/client.o

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

test: $(SHARED_LIB) $(TEST_EXECS)
	@echo -e "\nRunning tests...\n"
	@for test in $(TEST_EXECS); do \
		if $$test > /dev/null 2>&1; then \
			echo -e "$$test [PASS]"; \
		else \
			echo -e "$$test [FAIL]"; \
		fi \
	done
	@echo -e "\nAll tests completed."

$(TEST_BUILD_DIR)/%: $(TEST_DIR)/%.c | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(TEST_LDFLAGS)
	@echo "Compiled $(notdir $@) [OK]"

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

clean:
	rm -rf $(OBJ_DIR) $(SHARED_LIB) $(TEST_BUILD_DIR)

check: clean
.PHONY: all test clean check
