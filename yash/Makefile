CC = gcc
CFLAGS = -I./include -Wall -g
LDFLAGS = -lreadline

SRC_DIR = src
INCLUDE_DIR = include

SRCS = $(SRC_DIR)/yash.c $(SRC_DIR)/parse.c $(SRC_DIR)/run.c
OBJS = $(SRCS:.c=.o)

TARGET = yash

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(SRC_DIR)/*.o

.PHONY: all clean
