CC = gcc
CFLAGS = -Wall
RM=rm
LD=ld
TARGET = my_mpstat

.PHONY: clean all run

all: $(TARGET)
run: clean all

$(TARGET): my_mpstat.c
	$(CC) $(CFLAGS) -o $(TARGET) my_mpstat.c

clean:
	$(RM) -f $(TARGET)