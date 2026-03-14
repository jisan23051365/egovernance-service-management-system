CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c99 -Isrc
LDFLAGS =

TARGET  = egovernance
SRC_DIR = src
OBJ_DIR = build

SRCS    = $(SRC_DIR)/utils.c \
          $(SRC_DIR)/auth.c \
          $(SRC_DIR)/service_request.c \
          $(SRC_DIR)/tax_record.c \
          $(SRC_DIR)/tax_analytics.c \
          $(SRC_DIR)/main.c

OBJS    = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: all
	./$(TARGET)

tests:
	$(MAKE) -C tests

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	$(MAKE) -C tests clean
