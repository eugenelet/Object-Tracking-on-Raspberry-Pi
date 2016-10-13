RCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp,%,$(SRCS))
OBJS = $(SRCS:.cpp=.o)
TEMPS = $(SRCS:.cpp=.txt)
SRC = transmit.cpp receiver.cpp
OUT = car

CFLAGS = `pkg-config --cflags --libs opencv`
LDFLAGS = `pkg-config --libs opencv`
LIB = -lpthread -lpigpio -lrt -std=c++11

all: car

car: car.cpp
	g++ car.cpp GPIOClass.cpp $(SRC) $(LIB)  -o $(OUT)

clean:
	@rm -f $(PROGS) $(OBJS) $(TEMPS) $(OUT)
	@echo "Limpo!"
