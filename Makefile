#--------------------------------------------
# predefine rule
#--------------------------------------------
.PHONY: clean

#--------------------------------------------
# Tool configuration
#--------------------------------------------
CC = g++
LEVELDB = ~/Desktop/leveldb-1.15.0
LINKER = $(CC)
EXT = cpp
LIB = $(LEVELDB)/libleveldb.a
MKDIR = mkdir -p
CFLAGS += -I $(LEVELDB)/include -O2
LDFLAGS = -lpthread

#--------------------------------------------
# Path configuration
#--------------------------------------------
MODULES = $(wildcard src/*.$(EXT))
SOURCEDIR = src
BINDIR = bin
TARGET = main

#--------------------------------------------
# Suffix
#--------------------------------------------
$(BINDIR)/%.o : $(SOURCEDIR)/%.$(EXT)
	$(CC) -c $< -o $@ $(CFLAGS)

# Compiling main
OBJS := ${MODULES:src/%.$(EXT)=bin/%.o}

#--------------------------------------------
# target
#--------------------------------------------
all: mkdir $(OBJS)
	$(LINKER) -o $(BINDIR)/$(TARGET) $(OBJS) $(LIB) $(LDFLAGS)

mkdir: 
	$(MKDIR) $(BINDIR)

# Run
run:
	$(BINDIR)/$(TARGET)

# Cleaning compilation
clean:
	rm -rf $(BINDIR)/*.o
	rm -rf $(BINDIR)/*~
