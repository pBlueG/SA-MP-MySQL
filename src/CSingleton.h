#pragma once
#ifndef INC_CSINGLETON_H
#define INC_CSINGLETON_H


template<class T>
class CSingleton
{
protected:
	static T *m_Instance;

public:
	CSingleton() { }
	virtual ~CSingleton() { }

	inline static T *Get()
	{
		if (m_Instance == nullptr)
			m_Instance = new T;
		return m_Instance;
	}

	inline static void Destroy()
	{
		if (m_Instance != nullptr)
		{
			delete m_Instance;
			m_Instance = nullptr;
		}
	}
};

template <class T>
T* CSingleton<T>::m_Instance = nullptr;


#endif // INC_CSINGLETON_H
