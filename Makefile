CC=g++
CFLAGS = -std=c++11 -pthread

DEPS=barrier.h dirops.h fileops.h namegen.h nameset.h ops.h processopts.h taskresult.h log.h
OBJS=barrier.o dirops.o fileops.o fsmdbenchmark.o namegen.o nameset.o ops.o processopts.o log.o

TARGET=fsmdbench

all: $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	$(RM) $(TARGET) *.o
