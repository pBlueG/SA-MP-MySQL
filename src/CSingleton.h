#pragma once


template<class T>
class CSingleton
{
protected:
	static T *m_Instance;

public:
	CSingleton() = default;
	virtual ~CSingleton() = default;

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
