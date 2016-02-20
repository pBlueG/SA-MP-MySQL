#pragma once

#include <memory>
#include <functional>
#include <binary_log_types.h>

using std::shared_ptr;

typedef struct st_mysql MYSQL;
typedef unsigned long long my_ulonglong;

class CHandle;
class ISqlStatement;
class CQuery;
class CPreparedStmt;
class CPawnPreparedStmt;
class CCallback;
class CResultSet;
class COptions;


using Handle_t = CHandle *;
using HandleId_t = unsigned int;

using ISqlStmt_t = shared_ptr<ISqlStatement>;
using Query_t = shared_ptr<CQuery>;
using PrepStmt_t = shared_ptr<CPreparedStmt>;
using PawnPrepStmt_t = shared_ptr<CPawnPreparedStmt>;
using PawnPrepStmtId_t = unsigned int;

using Callback_t = shared_ptr<CCallback>;

using ResultSet_t = shared_ptr<CResultSet>;
using ResultSetId_t = unsigned int;

using OptionsId_t = unsigned int;

using DispatchFunction_t = std::function < void() >;
