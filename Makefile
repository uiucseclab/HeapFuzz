CXX = clang++
C = clang
LD = clang++

#CXXFLAGS = CXXFLAGS = -std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -Werror -pedantic
#LDFLAGS = -std=c++17 -stdlib=libc++  -ldl
LDFLAGS = 


BASE_DIR = $(shell pwd)
BIN_DIR = $(BASE_DIR)/bin
SRC_DIR = $(BASE_DIR)/src
OBJ_DIR = $(BASE_DIR)/obj
TEST_DIR = $(BASE_DIR)/test

CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++1z -g3
CFLAGS = -Wall -Wextra -g 

main.o: $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o obj/main.o -o $(OBJ_DIR)/main.o

exec.o: $(SRC_DIR)/exec.cpp $(SRC_DIR)/exec.hpp 
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/exec.cpp -o $(OBJ_DIR)/exec.o


control.so: $(SRC_DIR)/control.c
	$(C) $(CFLAGS) -shared -o $(BIN_DIR)/control.so -fPIC $(SRC_DIR)/control.c


all: main.o exec.o control.so
	$(LD) $(LDFLAGS) $(OBJ_DIR)/main.o $(OBJ_DIR)/exec.o -o $(BIN_DIR)/fuzzer

shim_test.o: $(TEST_DIR)/basic/shim_test.c
	$(C) $(CFLAGS) -c $(TEST_DIR)/basic/shim_test.c -o $(OBJ_DIR)/shim_test.o
shim_test: shim_test.o
	$(LD) $(LDFLAGS) $(OBJ_DIR)/shim_test.o -o $(TEST_DIR)/basic/shim_test
	

.PHONY: clean
clean:
	rm $(OBJ_DIR)/*.o $(BIN_DIR)/fuzzer

#Formats all your source code (TODO: agree to coding standard)
.PHONY: format
format:
	find src -name "*.cpp" | xargs clang-format -style=google -i 
	find src -name "*.hpp" | xargs clang-format -style=google -i 
