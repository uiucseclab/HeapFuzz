#Define PRoject Structure
BASE_DIR ?= $(shell pwd)
SRC_DIR ?= $(BASE_DIR)/src/
BIN_DIR ?= $(BASE_DIR)/bin/
OBJ_DIR ?= $(BASE_DIR)/bin/obj/
TARGET ?= weneedaname


#Build a list of all cpp/c/assembly files
SRCS := $(shell find $(SRC_DIR) -name *.cpp -or -name *.c -or -name *.S)
#Construct a list of all object names
OBJS := $(addsuffix .o,$(basename $(SRCS)))

#The superior compiler
CXX=clang++

#Debug
STD_FLAGS += -g3
#Allow for modern C++
STD_FLAGS += -std=c++11
#Enforce basic standards
STD_FLAGS +=  -Wall -Werror -Wpedantic  


#Optional flags
SAFE_FLAGS := -fsanitize=address

$(TARGET): $(OBJS)
	$(CXX) $(STD_FLAGS) $(OBJS) -o $(TARGET)


#Make sure clang doesn't try to build a file called clean
.PHONY: clean

clean:
	rm -rf $(BIN_DIR)

#Formats all your source code (TODO: agree to coding standard)
.PHONY: format
format:
	find . -name "*.cpp" | xargs clang-format -style=file -i



