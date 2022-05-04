CXX      := -c++
#CXXFLAGS := -Wall -Wextra -Werror -std=c++11
CXXFLAGS := -Wall -std=c++11 -O3
LDFLAGS  := -L/usr/lib -lstdc++ -lm
BUILD    := ./build
#OBJ_DIR  := $(BUILD)/objects
OBJ_DIR  := obj
APP_DIR  := bin
TARGET   := sim-riscv
INCLUDE  := -Iinclude
INCLUDE	 += -Ilib/elfio
INCLUDE	 += -Ilib/libargparse/src
SRC      := $(wildcard src/*.cpp)
SRC	 += $(wildcard lib/libargparse/src/*.cpp)			    


OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean debug release info

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

clean_all:
	-@rm -rvf $(OBJ_DIR)
	-@rm -rvf $(APP_DIR)

info:
	@echo "[*] Application dir: ${APP_DIR}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Sources:         ${SRC}         "
	@echo "[*] Objects:         ${OBJECTS}     "
	@echo "[*] Dependencies:    ${DEPENDENCIES}"


