CC=g++
CSTD=-std=c++0x
EXECUTABLE=vanet
LIBS=-lpthread -lrt
SOURCES=Vehicle.cpp

all: $(EXECUTABLE)

$(EXECUTABLE): clean
	clear
	$(CC) -o $(EXECUTABLE) $(SOURCES) $(CSTD) $(LIBS)

clean:
	rm -f *.txt $(EXECUTABLE)