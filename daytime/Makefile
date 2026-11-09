CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = daytime_client
SRC = daytime_client.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
