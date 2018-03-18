CXX = clang++
LD = clang++

#CXXFLAGS = CXXFLAGS = -std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -Werror -pedantic
LDFLAGS = -std=c++17 -stdlib=libc++  

BASE_DIR = $(shell pwd)
BIN_DIR = $(BASE_DIR)/bin
SRC_DIR = $(BASE_DIR)/src
OBJ_DIR = $(BASE_DIR)/obj

CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++17 -g3

main.o: $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o obj/main.o -o $(OBJ_DIR)/main.o

instrument.o: $(SRC_DIR)/instrument.cpp $(SRC_DIR)/instrument.hpp 
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/instrument.cpp -o $(OBJ_DIR)/instrument.o

snapshot.o: $(SRC_DIR)/snapshot.cpp $(SRC_DIR)/snapshot.hpp 
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/snapshot.cpp -o $(OBJ_DIR)/snapshot.o

all: main.o instrument.o snapshot.o
	$(LD) $(OBJ_DIR)/main.o $(OBJ_DIR)/Instrument.o $(OBJ_DIR)/snapshot.o -o $(BIN_DIR)/fuzzer


.PHONY: clean
clean:
	rm $(OBJ_DIR)/*.o $(BIN_DIR)/fuzzer

#Formats all your source code (TODO: agree to coding standard)
.PHONY: format
format:
	find src -name "*.cpp" | xargs clang-format -style=google -i 
	find src -name "*.hpp" | xargs clang-format -style=google -i 