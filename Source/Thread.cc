#include "WinException.h"
#include "Thread.h"

namespace ipop
{
namespace win
{

/*--
FUNCTION:
	Thread::Thread()	
PURPOSE:
	c'tor
PARAMETERS:
	Security - Pointer to a SECURITY_ATTRIBUTES structure that determines whether the returned handle can be inherited by child processes. If NULL, the handle cannot be inherited. Must be NULL for Windows 95 applications. 
	Stacksize - Stack size for new thread or 0 to uses the same value as the stack specified for the main thread.
	InitFlag - Initial state of new thread (0 for running or CREATE_SUSPENDED for suspended); use ResumeThread to execute the thread. 	
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
++*/
Thread::Thread(	SECURITY_ATTRIBUTES* Security, 
				unsigned int StackSize, 
				unsigned int InitFlag): 
					mSecurity(Security), 
					mStackSize(StackSize), 
					mInitFlag(InitFlag)
{
	mThreadHandle = NULL;
	mThreadID = 0;
	mArgList = NULL;
}
				
/*--
FUNCTION:
	Thread::~Thread()
PURPOSE:
	d'tor
PARAMETERS:
	None
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
++*/
Thread::~Thread() 
{
	if(mThreadHandle)
		CloseHandle(mThreadHandle);
}

/*--
FUNCTION:
	Thread::Start()
PURPOSE:
	Call this function to invoke the new thread of execution.
PARAMETERS:
	arg - passed to Execute()
RETURN VALUE:
	None
THROWS:
	exception - if failed to begin thread
COMMENTS:
++*/
void Thread::Start(void *arg)
{
	mArgList = arg;
	mThreadHandle = (HANDLE)_beginthreadex(	mSecurity, 
											mStackSize, 
											(unsigned (__stdcall*)(void*))Thread::EntryPoint, 
											static_cast<void*>(this), 
											mInitFlag, 
											(unsigned int*)&mThreadID);
	if( mThreadHandle == 0) return 
		throw WINEXCEPT("Failed to start thread - Failed on _beginthreadex");
	return;
}

/*--
FUNCTION:
	Thread::Resume() 
PURPOSE:
	resumes a thread created with the CREATE_SUSPENDED flag
PARAMETERS:
	
RETURN VALUE:
	If the function succeeds, the return value is the thread's previous suspend count.
THROWS:
	None
COMMENTS:
++*/
unsigned int Thread::Resume()
{
	return ResumeThread(mThreadHandle);
}

/*--
FUNCTION:
	Thread::Run()
PURPOSE:
	Invoke the child class implmentation of Execute()
PARAMETERS:
	None.
RETURN VALUE:
	int - the status code supplied by the child class implementation of Execute()
THROWS:
	None
COMMENTS:
++*/
int Thread::Run()
{
//	Setup();
	return Execute(mArgList);
}

/*--
FUNCTION:
	Thread::EntryPoint()
PURPOSE:
	This is the function that is passed to beginthreadex.
PARAMETERS:
	pThis -  self pointer to the object
RETURN VALUE:
	None
THROWS:
	None
COMMENTS: 
	static function to meet required prototype for beginthreadex()
++*/
unsigned int Thread::EntryPoint(void * pThis)
{
	Thread * pt = (Thread*)pThis;
	return (unsigned)pt->Run();
}

/*
 *Setup() pure virtual funtion
 *Derived class can do any setup needed here
 *virtual void Thread::Setup(){}
*/

/*
 *Execute() pure virtual function
 *Drived class code to be executed on new thread goes here
 *virtual int Thread::Execute(void* arg){}
*/

}
}