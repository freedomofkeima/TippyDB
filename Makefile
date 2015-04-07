#--------------------------------------------
# predefine rule
#--------------------------------------------
.PHONY: clean

#--------------------------------------------
# Tool configuration
#--------------------------------------------
LEVELDB =~/Desktop/leveldb-1.15.0
MONGODB =/usr/include/mongo
INCS_DIRS  =-I/usr/local/include/thrift -I /usr/include/boost
LEVELDB_INCS_DIRS =-I $(LEVELDB)/include
MONGODB_INCS_DIRS =-I $(MONGODB)
LIBS_DIRS  =-L/usr/local/lib 
LEVELDB_LIBS_DIRS =$(LEVELDB)/libleveldb.a
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   =-Wall -O2
LIBS       =-lthrift 
LEVELDB_LIBS =-lpthread
MONGODB_LIBS =-pthread -lmongoclient -lboost_thread-mt -lboost_system -lboost_rregex 

#--------------------------------------------
# Path configuration
#--------------------------------------------
SOURCEDIR = src
BINDIR = bin
DATABASEDIR = src/db

#--------------------------------------------
# Thrift generated files
#--------------------------------------------
GEN_SRC    = $(SOURCEDIR)/gen-cpp/dbservice_types.cpp \
             $(SOURCEDIR)/gen-cpp/DBService.cpp
GEN_INC    = -I$(SOURCEDIR)/gen-cpp

#--------------------------------------------
# target
#--------------------------------------------
default: mkdir server testcase local_testcase

mkdir:
	mkdir -p $(BINDIR)

server: $(SOURCEDIR)/DBServiceServer.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o $(BINDIR)/server ${GEN_INC} ${INCS_DIRS} ${LEVELDB_INCS_DIRS} $(DATABASEDIR)/*.cpp $(SOURCEDIR)/DBServiceServer.cpp ${GEN_SRC} ${LIBS_DIRS} ${LEVELDB_LIBS_DIRS} ${LIBS} ${LEVELDB_LIBS}

testcase: $(SOURCEDIR)/DBServiceTestcase.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o $(BINDIR)/testcase ${GEN_INC} ${INCS_DIRS} $(SOURCEDIR)/DBServiceTestcase.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

local_testcase: $(SOURCEDIR)/DBLocalTestcase.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o $(BINDIR)/local_testcase ${INCS_DIRS} ${LEVELDB_INCS_DIRS} $(SOURCEDIR)/DBLocalTestcase.cpp ${LIBS_DIRS} ${LEVELDB_LIBS_DIRS} ${LEVELDB_LIBS}

mongodb_local_testcase: $(SOURCEDIR)/MongoDBLocalTestcase.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o $(BINDIR)/mongodb_local_testcase ${INCS_DIRS} ${MONGODB_INCS_DIRS} $(SOURCEDIR)/MongoDBLocalTestcase.cpp ${LIBS_DIRS} ${MONGODB_LIBS}

#Run (Server)
run:
	$(BINDIR)/server

clean:
	rm -rf $(SOURCEDIR)/*.o
	rm -rf $(SOURCEDIR)/*.~
	rm -rf $(SOURCEDIR)/gen-cpp/*.o
	rm -rf $(SOURCEDIR)/gen-cpp/*.~
