#pragma once

#include <memory>
#include <functional>
#include <tuple>

using std::shared_ptr;
using std::unique_ptr;
using std::tuple;

class CHandle;
class CQuery;
class CCallback;
class CResultSet;
class CResult;
class COptions;
class COrm;


using Handle_t = CHandle *;
using HandleId_t = unsigned int;

using Query_t = shared_ptr<CQuery>;

using Callback_t = shared_ptr<CCallback>;

using ResultSet_t = shared_ptr<CResultSet>;
using Result_t = CResult *;
using ResultSetId_t = unsigned int;

using OptionsId_t = unsigned int;

using DispatchFunction_t = std::function < void() >;

using Orm_t = shared_ptr<COrm>;
using OrmId_t = unsigned int;
