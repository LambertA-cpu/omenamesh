
CFLAGS := -I../src
ASMFLAGS := -nostartfiles -no-pie -lc

BUILD_DIR := build
SRC_FILES := main.c client.c
OBJ_FILES := $(BUILD_DIR)/client.o
EXECUTABLE := $(BUILD_DIR)/main
ASM_SOURCE := node.S
ASM_EXEC := $(BUILD_DIR)/program


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES) main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ main.c $(OBJ_FILES)
	@echo "Running $(EXECUTABLE)..."
	@./$(EXECUTABLE)


$(BUILD_DIR)/client.o: client.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


assemble: $(ASM_EXEC)
	@echo "Running $(ASM_EXEC)..."
	@./$(ASM_EXEC)

$(ASM_EXEC): $(ASM_SOURCE) | $(BUILD_DIR)
	$(CC) $(ASMFLAGS) -o $@ $<


$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all assemble clean

