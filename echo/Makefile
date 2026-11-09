CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = echo_client
SRC = echo_client.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
