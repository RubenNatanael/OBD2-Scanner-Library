CXX := g++
CXXFLAGS := -g -Wall -Wextra -std=c++17 -Iinclude

TARGET := main

SRC := example/main.cpp \
       src/CustomSocket.cpp \
       src/GenerateFrames.cpp \
	   src/ReceiverFrames.cpp \
       src/Mode1Pids.cpp \
	   src/TroubleCodes.cpp \
	   src/OBD2Scanner.cpp

OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lfmt -lspdlog

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
