MySQL Plugin for San Andreas Multiplayer (SA:MP) [![Build Status](https://travis-ci.org/pBlueG/SA-MP-MySQL.svg?branch=master)](https://travis-ci.org/pBlueG/SA-MP-MySQL)
------------------------------------------------
*The best and most famous MySQL plugin for SA:MP out there!*

**This plugin allows you to use MySQL in PAWN. It's currently being developed by maddinat0r.**

How to install
--------------
Move *mysql.dll* (Windows) or *mysql.so* (Linux) to your `plugins/` directory. If you are on Windows you also have to move the *libmysql.dll* to your main server directory.
You'll have to edit the server configuration (*server.cfg*) as follows:
#### Windows
<pre>plugins mysql</pre>

#### Linux
<pre>plugins mysql.so</pre>

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
#### Windows
1. Install Microsoft Visual Studio C++ (2012 or newer, the Express version also works) and the [MySQL C Connector (32-bit)](http://dev.mysql.com/downloads/connector/c/)
2. Install the [boost libraries (version 1.55 or higher)](http://www.boost.org/users/download/)
3. Open the solution file with Visual Studio -> right click on the project -> Properties -> VC++ Directories, use *Release* as configuration and adjust the paths to the previously installed libraries
4. Build the solution with *Release* as configuration

#### Linux
1. Install these packages (names may vary throughout the distributions): `g++-multilib mysql-client libmysqlclient libmysqlclient-dev`
2. Install the [boost libraries (version 1.55 or higher)](http://www.boost.org/users/download/)
3. Navigate to the project root directory and execute `make`

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
