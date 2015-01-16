MySQL Plugin for San Andreas Multiplayer (SA:MP) [![Build Status](https://travis-ci.org/pBlueG/SA-MP-MySQL.svg?branch=master)](https://travis-ci.org/pBlueG/SA-MP-MySQL)
------------------------------------------------
*The best and most famous MySQL plugin for SA:MP out there!*

**This plugin allows you to use MySQL in PAWN. It's currently being developed by maddinat0r.**

How to install
--------------
1. Extract the content of the downloaded archive into the root directory of your SA-MP server.
2. Edit the server configuration (*server.cfg*) as follows:
   - Windows: `plugins mysql`
   - Linux: `plugins mysql.so`

F.A.Q.
------
Q: *I get a* `Failed (libmysqlclient_r.so.16: cannot open shared object file: No such file or directory)` *error, how do I fix this?*  
A: You don't have the MySQL client library installed. Install the "mysql-client" package.

Q: *The plugin still can't find the "libmysqlclient_r.so.16", what can I do now?*  
A: If that error still occurs, you are probably using a newer version of the MySQL client library. In that case use the `mysql_5.5.so`, which is linked to the libmysqlclient18 rather than the libmysqlclient16.

Q: *I get a* `Failed (plugins/mysql.so: symbol __cxa_pure_virtual, version libmysqlclient_16[...]` *error, is there any way to fix it?*  
A: That likely means that you are using a 64bit system and thus a 64bit libmysqlclient library. You'll have to use the static version of the plugin, the `mysql_static.so`.  

Q: *The plugin just fails to load on Windows, how can I fix this?*  
A: You have to install the Microsoft C++ redistributables ([2010 (x86)](http://www.microsoft.com/en-us/download/details.aspx?id=5555), [2010 SP1 (x86)](http://www.microsoft.com/en-us/download/details.aspx?id=8328) and [2012 (x86)](http://www.microsoft.com/en-us/download/details.aspx?id=30679)).

Q: *I get a ton of debug messages regarding connections even though I'm calling* `mysql_connect` *only once, why is that so?*  
A: That's because the plugin uses multiple direct database connections per connection handle. The number of direct connections (and thus the number of those log messages) is 2+pool_size.  

Build instruction
---------------
*Note*: The plugin has to be a 32-bit library; that means all required libraries have to be compiled in 32-bit and the compiler has to support 32-bit.
#### Windows
1. install a C++ compiler of your choice
2. install the [MySQL C Connector (version 6.1.5)](http://dev.mysql.com/downloads/connector/c/)
3. install the [Boost libraries (version 1.57 or higher)](http://www.boost.org/users/download/)
4. install [CMake](http://www.cmake.org/)
5. clone this repository
6. create a folder named `build` and execute CMake in there
7. build the generated project files with your C++ compiler

#### Linux
1. install a C++ compiler of your choice
2. install the appropriate MySQL client (version 5.5.40 or higher) through your package manager
3. install the [Boost libraries (version 1.57 or higher)](http://www.boost.org/users/download/)
4. install [CMake](http://www.cmake.org/)
5. clone this repository
6. create a folder named `build` and execute CMake in there (`mkdir build && cd build && cmake ..`)
7. build the generated project files with your C++ compiler

Thanks to
---------
- AndreT (testing/several tutorials)
- DamianC (testing reports)
- JernejL (testing/suggestions)
- krisk (testing/suggestions)
- Kye (coding support)
- maddinat0r (developing the plugin as of R8)
- Mow (compiling/testing/hosting)
- nemesis (testing)
- Sergei (testing/suggestions/wiki documentation)
- xxmitsu (testing/compiling)
