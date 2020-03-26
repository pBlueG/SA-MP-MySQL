# MySQL Plugin for San Andreas Multiplayer (SA:MP)

| Travis CI | AppVeyor | Total downloads | Latest release |
| :---: | :---: | :---: | :---: |
|  [![Build Status](https://travis-ci.org/pBlueG/SA-MP-MySQL.svg?branch=master)](https://travis-ci.org/pBlueG/SA-MP-MySQL)   |  [![Build status](https://ci.appveyor.com/api/projects/status/xssdxu7wp8l3q2mk/branch/master?svg=true)](https://ci.appveyor.com/project/maddinat0r/sa-mp-mysql/branch/master)  |  [![All Releases](https://img.shields.io/github/downloads/pBlueG/SA-MP-MySQL/total.svg?maxAge=86400)](https://github.com/pBlueG/SA-MP-MySQL/releases)  |  [![latest release](https://img.shields.io/github/release/pBlueG/SA-MP-MySQL.svg?maxAge=86400)](https://github.com/pBlueG/SA-MP-MySQL/releases) <br> [![Github Releases](https://img.shields.io/github/downloads/pBlueG/SA-MP-MySQL/latest/total.svg?maxAge=86400)](https://github.com/pBlueG/SA-MP-MySQL/releases)  |
-------------------------------------------------
*The best and most famous MySQL plugin for SA:MP out there!*

**This plugin allows you to use MySQL in PAWN. It's currently being developed by [maddinat0r](https://github.com/maddinat0r).**

How to install
--------------
1. Extract the content of the downloaded archive into the root directory of your SA-MP server.
2. Edit the server configuration (*server.cfg*) as follows:
   - Windows: `plugins mysql`
   - Linux: `plugins mysql.so`

F.A.Q.
------
Q: *I get a* `version GLIBCXX_3.4.15' not found` *error (or similar). How can I solve this?*
A: Update your system. If that still didn't work, you'll need to upgrade your Linux distribution to a version which provides the gcc 4.8 (or higher) compiler. For example, if you're on CentOS 6, which only has gcc 4.4, you'll need to upgrade to CentOS 7.

Q: *I get a* `Failed (libmysqlclient_r.so.18: cannot open shared object file: No such file or directory)` *error, how do I fix this?*
A: You don't have the MySQL client library installed. Install it through your package manager. Make sure you install the 32bit (i386, i686, etc) library, or else the plugin won't run.

Q: *I can't install the required libmysqlclient library on my Linux distribution. What do I do now?*
A: Use the `mysql_static.so` plugin file. It's statically linked to the libmysqlclient library.

Q: *I get a* `Failed (plugins/mysql.so: symbol __cxa_pure_virtual, version libmysqlclient_18[...]` *error, is there any way to fix it?*
A: That likely means that you are using a 64bit system and thus a 64bit libmysqlclient library. You'll have to either install the 32bit version of the MySQL client package or use the statically linked version of the plugin, the `mysql_static.so`.

Q: *The plugin fails to load on Windows, how can I fix this?*
A: You have to install these Microsoft C++ redistributables. You'll need the x86/32bit downloads.
   - [2010 (x86)](http://www.microsoft.com/en-us/download/details.aspx?id=5555)
   - [2010 SP1 (x86)](http://www.microsoft.com/en-us/download/details.aspx?id=8328)
   - [2012 (x86)](http://www.microsoft.com/en-us/download/details.aspx?id=30679)
   - [2015 (x86)](https://www.microsoft.com/en-US/download/details.aspx?id=48145)

Q: *I'm not on Windows 10 and the plugin still fails to load after installing all the redistributables. Is there a solution for this?*
A: Download the [universal Windows CRT](https://www.microsoft.com/en-US/download/details.aspx?id=48234). Requirements for this:
 - Windows 8.1 and Windows Server 2012 R2: [KB2919355](https://support.microsoft.com/en-us/kb/2919355)
 - Windows 7 and Windows Server 2008 R2: [Service Pack 1](https://support.microsoft.com/en-us/kb/976932)
 - Windows Vista and Windows Server 2008: [Service Pack 2](https://support.microsoft.com/en-us/kb/948465)

Q: *I get a ton of debug messages regarding connections even though I'm calling* `mysql_connect` *only once, why is that so?*
A: That's because the plugin uses multiple direct database connections per connection handle. The number of direct connections (and thus the number of those log messages) is `2 + pool_size`.

Build instruction
---------------
*Note*: The plugin has to be a 32-bit library; that means all required libraries have to be compiled in 32-bit and the compiler has to support 32-bit.
#### Windows
1. install a C++ compiler of your choice
2. install the [MySQL C Connector (version 6.1.6)](http://dev.mysql.com/downloads/connector/c/)
3. install the [Boost libraries (version 1.57 or higher)](http://www.boost.org/users/download/)
4. install [CMake](http://www.cmake.org/)
5. clone this repository
6. create a folder named `build` and execute CMake in there
7. build the generated project files with your C++ compiler

#### Linux
1. install a C++ compiler of your choice
2. install the appropriate MySQL client (version 5.5 or higher) through your package manager
3. install the [Boost libraries (version 1.57 or higher)](http://www.boost.org/users/download/)
4. install [CMake](http://www.cmake.org/)
5. clone this repository
6. create a folder named `build` and execute CMake in there (`mkdir build && cd build && cmake ..`)
7. build the generated project files with your C++ compiler

Thanks to
---------
- AndreT (testing/several tutorials)
- DamianC (testing reports)
- IstuntmanI (testing)
- JernejL (testing/suggestions)
- Konstantinos (testing)
- krisk (testing/suggestions)
- kurta999 (testing)
- Kye (coding support)
- maddinat0r (developing the plugin as of R8)
- Mow (compiling/testing/hosting)
- nemesis (testing)
- Sergei (testing/suggestions/wiki documentation)
- xxmitsu (testing/compiling)
