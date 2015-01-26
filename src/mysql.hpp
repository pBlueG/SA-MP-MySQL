#pragma once

#ifdef WIN32
# include <WinSock2.h>
#endif

#ifdef LINUX
# include <mysql/mysql.h>
#else
# include <mysql.h>
#endif
