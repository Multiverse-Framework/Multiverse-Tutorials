CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11 -Wl,-rpath,lib -Iinclude

SRC_DIR = src
BIN_DIR = bin
LIB_DIR = lib

TARGET = $(BIN_DIR)/main

MYLIB = my_multiverse_client

all: $(TARGET) $(LIB_DIR)/lib$(MYLIB).so

$(TARGET): $(LIB_DIR)/lib$(MYLIB).so
	$(CC) $(CFLAGS) -Llib -o $(TARGET) $(SRC_DIR)/main.cpp -l$(MYLIB) -lmultiverse_client_json -ljsoncpp -pthread

$(LIB_DIR)/lib$(MYLIB).so:
	$(CC) $(CFLAGS) -c -o $(LIB_DIR)/$(MYLIB).o $(SRC_DIR)/$(MYLIB).cpp
	$(CC) -shared -o $(LIB_DIR)/lib$(MYLIB).so $(LIB_DIR)/$(MYLIB).o
	rm -f $(LIB_DIR)/$(MYLIB).o

clean:
	rm -f $(BIN_DIR)/*