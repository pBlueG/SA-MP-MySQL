#pragma once

#include "CSingleton.h"
#include "sdk.h"

#include <string>
#include <queue>
#include <functional>
#include <stack>
#include <unordered_set>
#include <boost/variant.hpp>
#include <system_error> //TODO: maybe selfmade error class?

using std::string;
using std::queue;
using std::function;
using std::stack;
using std::unordered_set;
using boost::variant;
using std::error_condition;


class CCallback 
{
public: //type definitions
	using ParamList_t = stack<variant<cell, string>>;


	enum class Errors
	{
		EMPTY_NAME,
		EMPTY_PARAMETERS,
		INVALID_AMX,
		NOT_FOUND,
		INVALID_PARAM_OFFSET,
		INVALID_FORMAT_SPECIFIER,

		_NUM_ERRORS
	};

	class error_category : public std::error_category
	{
	public:
		const char* name() const
		{
			return "Callback";
		}
		string message(int ev) const;
	};

public: //constructor / destructor
	CCallback(AMX *amx, int cb_idx, ParamList_t &&params) :
		m_AmxInstance(amx),
		m_AmxCallbackIndex(cb_idx),
		m_Params(params)
	{

	}
	~CCallback() = default;


private: //variables
	function<void()>
		m_PreExecute,
		m_PostExecute;

	AMX *m_AmxInstance = nullptr;
	int m_AmxCallbackIndex = -1;

	ParamList_t m_Params;

	
public: //functions
	inline void OnPreExecute(decltype(m_PreExecute) &&func)
	{
		m_PreExecute = func;
	}
	inline void OnPostExecute(decltype(m_PostExecute) &&func)
	{
		m_PostExecute = func;
	}

	bool Execute();
	
};


class CCallbackManager : public CSingleton<CCallbackManager>
{
	friend class CSingleton<CCallbackManager>;
private: //constructor / destructor
	CCallbackManager() = default;
	~CCallbackManager() = default;


private: //variables
	queue<CCallback *> m_Callbacks;
	unordered_set<const AMX *> m_AmxInstances;


public: //functions
	CCallback *Create(AMX *amx, string name, string format, cell *params, cell param_offset,
		error_condition &error);
	inline bool IsValidAmx(const AMX *amx)
	{
		return m_AmxInstances.count(amx) == 1;
	}

	void AddAmx(const AMX *amx)
	{
		m_AmxInstances.insert(amx);
	}
	void RemoveAmx(const AMX *amx)
	{
		m_AmxInstances.erase(amx);
	}
	void Process();

};


namespace std
{
	template<>
	struct is_error_condition_enum<CCallback::Errors> : public true_type {};
}
