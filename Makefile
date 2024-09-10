CC = gcc
CFLAGS = -I./include -Wall -g
LDFLAGS = -lreadline

SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include

SRCS = $(SRC_DIR)/yash.c $(SRC_DIR)/parse.c
OBJS = $(SRCS:.c=.o)

TARGET = $(BIN_DIR)/yash

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(SRC_DIR)/*.o

.PHONY: all clean
