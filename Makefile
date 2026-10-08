CC = gcc

CFLAGS = -Wall -Wextra -std=c11

LDFLAGS = -lraylib \
           -lX11 \
           -lXrandr \
           -lXinerama \
           -lXi \
           -lXcursor \
           -lGL \
           -lm \
           -lpthread \
           -ldl \
           -lrt

TARGET = main

all:
	$(CC) main.c -o $(TARGET) $(CFLAGS) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)