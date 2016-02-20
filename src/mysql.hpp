#pragma once

#ifdef WIN32
# include <WinSock2.h>
# undef ERROR
#endif

#ifdef LINUX
# include <mysql/mysql.h>
#else
# include <mysql.h>
#endif
