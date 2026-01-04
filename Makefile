TARGET := IOCFBootleg
SRC_C  := src/CoreFoundation/*.c src/IOKit/*.c
SRC_H  := src/CoreFoundation/*.h src/device/*.h src/*.h include/CoreFoundation/*.h include/IOKit/*.h include/System/libkern/*.h
FLAGS  := -std=gnu17 -Wall -O3 -Wno-unused-but-set-variable -isystem include -isystem src

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).dll
else
    TARGET := $(TARGET).so
endif


.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC_C) $(SRC_H)
	$(CC) -shared -o $@ $(SRC_C) $(FLAGS) $(CFLAGS)

clean:
	rm -f $(TARGET)
