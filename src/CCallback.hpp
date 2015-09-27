#pragma once

#include "CSingleton.hpp"
#include "sdk.hpp"

#include <string>
#include <queue>
#include <functional>
#include <list>
#include <unordered_set>
#include <tuple>
#include <boost/any.hpp>

using std::string;
using std::queue;
using std::function;
using std::list;
using std::unordered_set;
using std::tuple;

#include "CError.hpp"
#include "types.hpp"


class CCallback
{
public: //type definitions
	using ParamList_t = list<tuple<char, boost::any>>;

	enum class Error
	{
		NONE,
		INVALID_AMX,
		INVALID_PARAMETERS,
		INVALID_PARAM_OFFSET,
		INVALID_FORMAT_SPECIFIER,
		EMPTY_NAME,
		NOT_FOUND,
		EXPECTED_ARRAY_SIZE,
		INVALID_ARRAY_SIZE,
		NO_ARRAY_SIZE,
	};
	static const string ModuleName;

public: //constructor / destructor
	CCallback(AMX *amx, int cb_idx, ParamList_t &&params) :
		m_AmxInstance(amx),
		m_AmxCallbackIndex(cb_idx),
		m_Params(params)
	{ }
	~CCallback() = default;

private: //variables
	AMX *m_AmxInstance = nullptr;
	int m_AmxCallbackIndex = -1;

	ParamList_t m_Params;
	
public: //functions
	bool Execute();

public: //factory function
	static Callback_t Create(AMX *amx, string name, string format, 
		cell *params, cell param_offset, CError<CCallback> &error);
};


class CCallbackManager : public CSingleton<CCallbackManager>
{
	friend class CSingleton<CCallbackManager>;
private: //constructor / destructor
	CCallbackManager() = default;
	~CCallbackManager() = default;


private: //variables
	unordered_set<const AMX *> m_AmxInstances;


public: //functions
	inline bool IsValidAmx(const AMX *amx)
	{
		return m_AmxInstances.count(amx) == 1;
	}

	inline void AddAmx(const AMX *amx)
	{
		m_AmxInstances.insert(amx);
	}
	inline void RemoveAmx(const AMX *amx)
	{
		m_AmxInstances.erase(amx);
	}

};
