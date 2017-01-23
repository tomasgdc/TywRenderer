#pragma once
#include <memory>
//Singleton pattern with C+11 std::unique_ptr

template<class T>
class Singleton
{
public:
	static T& getInstance()
	{
		if (!m_instance) {
			m_instance = std::unique_ptr<T>(std::move(new T));
		}
		return *m_instance;
	}
private:
	static std::unique_ptr<T> m_instance;
};

template<class T> std::unique_ptr<T> Singleton<T>::m_instance;