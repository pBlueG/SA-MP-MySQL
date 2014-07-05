GPP=g++ -m32
GCC=gcc -m32


COMPILE_FLAGS = -c -O3 -w -fPIC -DLINUX -Wall -I libs/
LIBRARIES = -pthread -lrt -Wl,-Bstatic -lboost_thread -lboost_chrono -lboost_date_time -lboost_system -lboost_atomic -Wl,-Bdynamic


all: compile dynamic_link static_link clean
dynamic: compile dynamic_link clean
static: compile static_link clean

compile:
	@mkdir -p bin
	@echo Compiling plugin..
	@ $(GPP) $(COMPILE_FLAGS) -std=c++0x src/*.cpp
	@echo Compiling plugin SDK..
	@ $(GPP) $(COMPILE_FLAGS) libs/sdk/*.cpp
	@ $(GCC) $(COMPILE_FLAGS) libs/sdk/amx/*.c

dynamic_link:
	@echo Linking \(dynamic\)..
	@ $(GPP) -O2 -fshort-wchar -shared -o "bin/mysql.so" *.o -lmysqlclient_r $(LIBRARIES)

static_link:
	@echo Linking \(static\)..
	@ $(GPP) -O2 -fshort-wchar -shared -o "bin/mysql_static.so" *.o -Wl,-Bstatic -lmysqlclient_r -Wl,-Bdynamic $(LIBRARIES)

clean:
	@ rm -f *.o
	@echo Done.
