CC=g++
CFLAGS = -std=c++11 -pthread -g -O3
LFLAGS = -lm

DEPS=barrier.h dirops.h fileops.h namegen.h nameset.h ops.h processopts.h taskresult.h log.h stats.h execworkload.h
OBJS=barrier.o dirops.o fileops.o fsmdbenchmark.o namegen.o nameset.o ops.o processopts.o log.o stats.o execworkload.o

TARGET=fsmdbench

all: $(TARGET)

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LFAGS) -o $@ $(OBJS)

clean:
	$(RM) $(TARGET) *.o
