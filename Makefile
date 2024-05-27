CC = g++
CFLAGS = -std=c++11 -Wall
ODIR = ./usr/bin/
TARGET = $(ODIR)server
SRCS = CSALab2.cpp MyServer.cpp
OPTS_FOR_TEST=-lcheck -lpthread -lrt -lsubunit -lm
HEADERS_DIR = hv
HEADERS = $(wildcard $(HEADERS_DIR)/*.h)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

tets:
	$(CC) $(CFLAGS) Test.cpp MyServer.cpp -o ./test $(OPTS_FOR_TEST)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean