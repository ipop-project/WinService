/**************************************************************************
						WatchDog
					-------------------
	description			: The WatchDog class which starts, stops and monitors tincan and controller. 
	
***************************************************************************/  

#pragma once
#include <string>
#include "ClientUDPSocket.h"
using namespace std;

namespace ipop
{
	namespace win
	{
		class WatchDog
		{
		public:
			WatchDog();
			~WatchDog();

			void StartIPoPProcesses();
			void TerminateIPoPProcesses();
			void CheckHealth();
		private:

			void CheckControllerHealth(bool & Status);
			void CheckTincanHealth(bool & Status);
			bool IsServiceStateGood();
			
			PROCESS_INFORMATION mCtrlerPI, mTincanPI;
			unsigned int mFailureCount;
			short mTincanPort;
			short mControllerPort;
			string mTincanAddress;
			string mControllerAddress;
			ADDRESS_FAMILY mAF;
		};

	}
}