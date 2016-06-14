CC=g++
#CFLAGS = -std=c++11 -pthread -g -O3
CFLAGS = -pthread -g -O3
LFLAGS = -lm -lrt

DEPS=barrier.h dirops.h fileops.h namegen.h nameset.h ops.h processopts.h taskresult.h log.h stats.h execworkload.h communication.h util.h
OBJS=barrier.o dirops.o fileops.o fsmdbenchmark.o namegen.o nameset.o ops.o processopts.o log.o stats.o execworkload.o communication.o util.o

TARGET=fsmdbench

all: $(TARGET)

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $(OBJS)

test: testcases/*.cpp communication.cpp communication.h barrier.cpp barrier.h log.cpp log.h util.cpp util.h
	$(CC) $(CFLAGS) $(LFAGS) -o testbarrier testcases/testbarrier.cpp communication.cpp barrier.cpp util.cpp

clean:
	$(RM) $(TARGET) *.o
