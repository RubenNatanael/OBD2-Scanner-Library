CXX := g++
BASE_FLAGS := -g -Wall -Wextra -std=c++17 -Iinclude

LIB_NAME := libobd2.a
LIB_SRC := src/CustomSocket.cpp \
           src/GenerateFrames.cpp \
           src/ReceiverFrames.cpp \
           src/Mode1Pids.cpp \
           src/Mode9Pids.cpp \
           src/TroubleCodes.cpp \
           src/OBD2Scanner.cpp
LIB_OBJ := $(LIB_SRC:.cpp=.o)

SIMPLE := main
SIMPLE_SRC := example/main.cpp

QT_EXAMPLE := qt_example
QT_SRC := example/qt_main.cpp
QT_FLAGS := $(shell pkg-config --cflags Qt6Widgets)
QT_LIBS := $(shell pkg-config --libs Qt6Widgets)

all: simple

# Build the library (common)
$(LIB_NAME): $(LIB_OBJ)
	ar rcs $@ $^

# Build console example (NO Qt)
simple: $(LIB_NAME) $(SIMPLE_SRC:.cpp=.o)
	$(CXX) $(BASE_FLAGS) -o $(SIMPLE) $^ -L. -lobd2 -lfmt -lspdlog

# Build Qt example (needs Qt includes/libs)
qt: $(LIB_NAME) $(QT_SRC:.cpp=.o)
	$(CXX) $(BASE_FLAGS) $(QT_FLAGS) -o $(QT_EXAMPLE) $^ -L. -lobd2 -lfmt -lspdlog $(QT_LIBS)

# Generic object rule
%.o: %.cpp
	$(CXX) $(BASE_FLAGS) -c $< -o $@

# Special rule for Qt source file (needs Qt flags)
$(QT_SRC:.cpp=.o): $(QT_SRC)
	$(CXX) $(BASE_FLAGS) $(QT_FLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJ) $(SIMPLE_SRC:.cpp=.o) $(QT_SRC:.cpp=.o) $(SIMPLE) $(QT_EXAMPLE) $(LIB_NAME)

.PHONY: all clean simple qt
