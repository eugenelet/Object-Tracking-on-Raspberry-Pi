RCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp,%,$(SRCS))
OBJS = $(SRCS:.cpp=.o)
TEMPS = $(SRCS:.cpp=.txt)
OUT = controller
SRC = src/transmit.cpp src/receiver.cpp src/getChar.cpp src/kbhit.cpp src/controller.cpp \
	src/computeDescriptor.cpp src/createDoG.cpp src/detectKeypoints.cpp src/drawKeyPoints.cpp \
	 src/filterKeyPoints.cpp src/match.cpp src/debug.cpp src/computeSift.cpp src/trackObject.cpp

CFLAGS = -O3 -lpthread `pkg-config --cflags --libs opencv`
LDFLAGS = `pkg-config --libs opencv`


all: controller


controller: $(SRC)
	g++ -g $(SRC) $(CFLAGS)  -o $(OUT)

clean:
	@rm -f $(PROGS) $(OBJS) $(TEMPS) $(OUT)
	@echo "Limpo!"
