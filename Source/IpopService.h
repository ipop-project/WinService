/**************************************************************************
						IpopService
					-------------------
	description			: The IPoP Service class. 
	
***************************************************************************/  

#pragma once
#include "WinService.h"
#include "WatchDog.h"

namespace ipop
{
	namespace win
	{

		class IpopService :
			virtual public WinService
		{
		private:
			static BOOL __stdcall ControlHandler(DWORD CtrlType);
			HANDLE mSvcExitEv;
			WatchDog mWDog;

		protected:
			void Init();
			void Run();
			void OnStop();
			void OnShutdown();
			void OnPause();
			void OnContinue();
			void HandleUserDefined(
				unsigned int Control
				);
		public:
			DECLARE_SERVICE(IpopService)
			IpopService(
				const wstring & SvcName,
				const wstring & DisplayName,
				unsigned int SvcType
				);
			~IpopService();
			void Debug();
		};

	}
}