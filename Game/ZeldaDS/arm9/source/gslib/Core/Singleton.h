#ifndef SINGLETON_H
#define SINGLETON_H

// Base class for Singletons
// Ideally, T's constructor should be private and declare
// friendship with Singleton<T>
template <typename T>
class Singleton
{
public:

	//@TODO: Add Create/Destroy functions to control memory allocation rather
	// than use lazy creation.

	static T& Instance()
	{
		static T instance;
		return instance;
	}
};

#endif // SINGLETON_H
