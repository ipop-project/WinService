/***************************************************************************
					Thread			
			------------------------
	description		: Allows an easy implementation of object-based multithreading
	Thread is an abstract wrapper class for win32 threads and hence is not meant
	to be instanciated. Instead derive a child class from it and instanciate
	that class. It is reccomened that each instance of the dervied class be
	run as only one thread i.e., the Start function should be invoked only
	once on each object. However there is nothing to preclude you from invoking
	Start() multiple times, but the validity of the logic will depend on the
	implemetation of your derived class.
	The derived  class will be responsible for implementing Setup() and
	Execute() as they are pure virtual functions. The implementation of
	Setup() is optional an can be left simply as a stub. It is invoked
	before Execute() to provide some added functionality/value to the programmer.
	Execute if the logic you wish to execute on this new thread. It would be
	nonsensical to leave Execute() as a stub;
 ***************************************************************************/

#if !defined(THREAD_H)
#define THREAD_H

#include <windows.h>
#include <process.h>

namespace ipop
{
namespace win
{

class Thread
{
public:
	//Thread();
	Thread(SECURITY_ATTRIBUTES* Security = NULL, unsigned int StackSize = 0, unsigned int InitFlag = 0);
	virtual ~Thread();
	void Start(void *arg);
	HANDLE& ThreadHandle(){return mThreadHandle;}
	unsigned int Resume();
protected:
	HANDLE mThreadHandle;
	unsigned int mThreadID;
	void * mArgList;
	SECURITY_ATTRIBUTES* mSecurity; 
	unsigned int mStackSize;
	unsigned int mInitFlag;

	int Run();
	static unsigned int EntryPoint(void*);
	//virtual void Setup() = 0;
	virtual int Execute(void*) = 0;
private:

};

}
}
#endif
