#include "Synchronized.h"

namespace ipop
{
namespace win
{

Mutex::Mutex ()
{
	mutex = ::CreateMutex(0, 0, 0);
}

Mutex::~Mutex ()
{
	::CloseHandle(mutex);
}

void Mutex::lock ()
{
	if (::WaitForSingleObject(mutex, INFINITE) == WAIT_ABANDONED){}
}

void Mutex::unlock ()
{
	if (!::ReleaseMutex(mutex)){}
}


CriticalSection::CriticalSection () 
{
	InitializeCriticalSection(&cs);
}

CriticalSection::~CriticalSection () 
{
	DeleteCriticalSection(&cs);
}

void CriticalSection::lock ()
{
	EnterCriticalSection(&cs);
}

void CriticalSection::unlock ()
{
	LeaveCriticalSection(&cs);
}


Synchronized::Synchronized (
	Lock & object) : mObject(object)
{ 
	mObject.lock(); 
}

Synchronized::~Synchronized()
{ 
	mObject.unlock(); 
}

}
}