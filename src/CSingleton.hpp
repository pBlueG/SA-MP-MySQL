#pragma once

#ifdef WIN32
	#pragma warning (disable: 4251)
	#if defined(DLL_EXPORTS)
		#define DLL_API __declspec(dllexport)
	#else
		#define DLL_API __declspec(dllimport)
	#endif
#else
	#define DLL_API
#endif

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
