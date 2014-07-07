
/**************************************************************************
                          WinService
                      -------------------
    description			: Win32 Service API wrapper class.
	Wrappers for a few useful synchronization primitives. Simply instantiate 
	a Synchronized obejct with a Mutex or CriticalSection. All the code in the
	scope of that object will be serialized on your Lock.
		1. Lock - Abstract base class
		2. Mutex - Mutex implementation of Lock
		3. CriticalSection - CriticalSection implelmentation of Lock
		4. Synchronized - Uses 2 or 3 to serialize a block of code
***************************************************************************/     

#pragma once
#include <Windows.h>

namespace ipop
{
namespace win
{

	class Lock
	{
	public:
		virtual void lock() = 0;
		virtual void unlock() = 0;
	};

	class Mutex :
		virtual public Lock
	{
	public :
		Mutex ();

		~Mutex ();

		void lock ();

		void unlock ();

	private:
		HANDLE mutex;
	};

	class CriticalSection : 
		virtual public Lock
	{
	public:
		CriticalSection () ;

		~CriticalSection ();

		void lock ();

		void unlock ();

	private:
		CRITICAL_SECTION	cs;
	};

	class Synchronized 
	{
	public:
		Synchronized (Lock & object);

		~Synchronized();

	protected:
		Lock & mObject;
	};
}
}