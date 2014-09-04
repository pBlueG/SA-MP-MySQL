#pragma once

#include "CSingleton.h"

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;


class CHandle
{
	friend class CHandleFactory;
public: //type definitions
	using Id_t = unsigned int;


private: //constructor / deconstructor
	CHandle(Id_t id) :
		m_Id(id)
	{ }
	~CHandle() = default;


private: //variables
	Id_t m_Id;


public: //functions
	inline Id_t GetId() const
	{
		return m_Id;
	}

};

class CHandleFactory : public CSingleton<CHandleFactory>
{
	friend class CSingleton<CHandleFactory>;
private: //constructor / deconstructor
	CHandleFactory() = default;
	~CHandleFactory() = default;


private: //variables
	unordered_map<CHandle::Id_t, CHandle *> m_Handles;


public: //functions
	CHandle *Create(string host, string user, string pass, string db, 
		size_t port, size_t pool_size, bool reconnect);
	bool Destroy(CHandle * handle);
};
