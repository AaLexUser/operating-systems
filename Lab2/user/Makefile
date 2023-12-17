CC = gcc
CFLAGS = -Wall
RM = rm
LD = ld
TARGET = my_mpstat

.PHONY: clean all run

all: $(TARGET)
run: clean all

$(TARGET): common.o my_mpstat.o
	$(CC) -o $(TARGET) common.o my_mpstat.o

common.o: common.c
	$(CC) $(CFLAGS) -c common.c

my_mpstat.o: my_mpstat.c
	$(CC) $(CFLAGS) -c my_mpstat.c
clean:
	$(RM) -f $(TARGET) common.o my_mpstat.o
	