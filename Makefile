CC = g++
CFLAGS = -Wall -Werror -Wextra
CPATH = 
TARGET = terminal

all: clang clean $(TARGET)

$(TARGET): terminal.cc
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET)

clang:
	clang-format -i --verbose *.cc