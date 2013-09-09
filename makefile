GPP=g++ -m32
GCC=gcc -m32


COMPILE_FLAGS=-c -O3 -w -fPIC -DLINUX -Wall -Isrc/SDK/amx/ -Isrc/ -DBOOST_THREAD_DONT_USE_CHRONO
BOOST_LIB_DIR=./src/boost_lib


all: compile dynamic_link static_link clean
dynamic: compile dynamic_link clean
static: compile static_link clean

compile:
	@echo Compiling plugin..
	@ $(GPP) $(COMPILE_FLAGS) src/*.cpp
	@echo Compiling plugin SDK..
	@ $(GPP) $(COMPILE_FLAGS) src/SDK/*.cpp
	@ $(GCC) $(COMPILE_FLAGS) src/SDK/amx/*.c
	@echo Compiling boost libraries..
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/date_time/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/system/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/thread/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/thread/pthread/*.cpp

dynamic_link:
	@echo Linking \(dynamic\)..
	@ $(GPP) -O2 -fshort-wchar -shared -o "bin/mysql.so" *.o -lmysqlclient_r -pthread -lrt

static_link:
	@echo Linking \(static\)..
	@ $(GPP) -O2 -fshort-wchar -shared -o "bin/mysql_static.so" *.o ./src/mysql_lib/libmysqlclient_r.a -pthread -lrt

clean:
	@ rm -f *.o
	@echo Done.
