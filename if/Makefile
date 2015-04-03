INCS_DIRS  = -I/usr/local/include/thrift -I /usr/include/boost
LIBS_DIRS  =-L/usr/local/lib
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   =-Wall -O2
LIBS       =-lthrift

GEN_SRC    = ./gen-cpp/SharedService.cpp  \
             ./gen-cpp/shared_types.cpp   \
             ./gen-cpp/tutorial_types.cpp \
             ./gen-cpp/Calculator.cpp
GEN_INC    = -I./gen-cpp

default: server client

server: CppServer.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o ../bin/CppServer ${GEN_INC} ${INCS_DIRS} CppServer.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

client: CppClient.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o ../bin/CppClient ${GEN_INC} ${INCS_DIRS} CppClient.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

clean:
	rm -rf *.o
	rm -rf *.~
	rm -rf gen-cpp/*.o
	rm -rf gen-cpp/*.~
