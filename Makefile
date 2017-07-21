#Copyright (C) 2015 Zong Wenbo

TARGET = libthreadpool.a
#CC  = gcc
AR = ar 
ARFLAGS = -rv
#CPPC = clang++-3.5 -std=c++14
CPPC = g++ -std=c++14
CFLAGS = -g -D_GNU_SOURCE -Wall -Iinclude -DVERBOSE -D NUMCORES=10
LIBS= -lpthread 
ALL_LIBS=$(LIBS) 
EXT=cc

.PHONY: default all clean

default: $(TARGET)
all: default

OBJ_DIR=obj

OBJECTS = $(patsubst %.$(EXT), obj/%.o, $(wildcard *.$(EXT)))
HEADERS = $(wildcard *.h)
HEADERS += $(wildcard *.hpp)

$(OBJ_DIR):
	mkdir -p ${OBJ_DIR}

obj/%.o: src/%.${EXT} $(HEADERS)
	$(CPPC) $(CFLAGS) -c $< -o $@

obj/%.o: %.${EXT} $(HEADERS)
	$(CPPC) $(CFLAGS) -c $< -o $@
			
.PRECIOUS: $(TARGET) $(OBJECTS) 

$(TARGET): $(OBJ_DIR) $(OBJECTS)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJECTS)  

clean:
	-rm -f obj/*.o
	-rm -f *.o
	-rm -f $(TARGET)
