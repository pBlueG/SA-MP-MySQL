MySQL Plugin for San Andreas Multiplayer (SA:MP)
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

Compiling notes
---------------
#### Windows
You'll need Microsoft Visual C++ and the boost libraries to compile the plugin. Open the project file, adjust the include and library paths (for the boost library), select *Release* at the top and compile the project solution.

#### Linux
You'll need *g++-multilib*, *mysql-client* and the *boost* libraries. Once you have all the required libraries, navigate to the project's folder and type `make`.

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
