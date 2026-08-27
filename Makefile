CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS := -lsqlite3

SRC_DIR := src
OBJ_DIR := bin
TARGET := resource_tracker

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run test init-db

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "\033[32m[BUILD SUCCESS]\033[0m Binary created: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

init-db: $(TARGET)
	./$(TARGET) --init-db

test: $(TARGET)
	./$(TARGET) --test

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) college_resources.db
	@echo "\033[33m[CLEAN]\033[0m Binaries and test database removed."
