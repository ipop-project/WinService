/**************************************************************************
						ServiceConfig
					-------------------
	description			: Loads and stores the service parameters. 
	
***************************************************************************/  

#pragma once
#include <string>
using namespace std;

namespace ipop
{
	namespace win
	{
		class ServiceConfig
		{
		public:
			ServiceConfig();
			~ServiceConfig();
			// Loads the service's configurable parameters
			int LoadServiceConfiguration();
			const wstring & GetIPoPPath() const;
			const wstring & GetPythonPath() const;
			const string & GetControllerRequest() const;
			const string & GetTincanRequest() const;
			size_t GetMaxStateMsgLen() const;
			size_t GetMaxSvcStartAttempts() const;
		private:
			void QueryPythonPath(
				wstring const & Version
				);

			void QueryIPopPath();

			wstring mIPopPath,
				mPythonPath; //path includes trailing slash '\'
			string mControllerRequest;
			string mTincanRequest;
			size_t mMaxStateMsgLen;
			size_t mMaxSvcStartAttempts;
		};

	}
}