#include <memory>
#include "EventLog.h"
#include "SystemInfo.h"
#include "WinException.h"
#include "ServiceConfig.h"

namespace ipop
{
namespace win
{

extern EventLog EventLogger;
extern SystemInfo SysInfo;

ServiceConfig::ServiceConfig() :
	mControllerRequest("\x2\x1{\"type\":\"echo_request\"}"),
	mTincanRequest("\x2\x1{\"m\":\"get_state\",\"uid\":\"0000\"}"),
	mMaxStateMsgLen(512),
	mMaxSvcStartAttempts(15)
{
}


ServiceConfig::~ServiceConfig()
{
}

// Loads the service's configurable parameters
int 
ServiceConfig::LoadServiceConfiguration()
{
	QueryPythonPath(L"2.7");
	QueryIPopPath();
	return 0;
}

const string & 
ServiceConfig::GetControllerRequest() const
{
	return mControllerRequest;
}

const string & 
ServiceConfig::GetTincanRequest() const
{
	return mTincanRequest;
}

const wstring & 
ServiceConfig::GetIPoPPath() const
{
	return mIPopPath;
}

const wstring & 
ServiceConfig::GetPythonPath() const
{
	return mPythonPath;
}

size_t 
ServiceConfig::GetMaxStateMsgLen() const
{
	return mMaxStateMsgLen;
}

size_t 
ServiceConfig::GetMaxSvcStartAttempts() const
{
	return mMaxSvcStartAttempts;
}

void 
ServiceConfig::QueryPythonPath(
	wstring const & Version)
{
	HKEY regkey;
	wstring regpath;
	unsigned long type = 0;
	unsigned long len = sizeof(BYTE)*MAX_PATH;
	unique_ptr<BYTE[]> buf = make_unique<BYTE[]>(len);
	memset(buf.get(), 0x0, len);
	wchar_t * path = reinterpret_cast<wchar_t*>(buf.get());


	if (SysInfo.IsWOW64Process())
	{
		regpath = L"SOFTWARE\\Wow6432Node\\Python\\PythonCore\\";
		regpath.append(Version).append(L"\\InstallPath");
	}
	else
	{
		regpath = L"SOFTWARE\\Python\\PythonCore\\";
		regpath.append(Version).append(L"\\InstallPath");
	}

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, regpath.c_str(), 0, KEY_READ, &regkey))
	{
		EventLogger.LogEvent(IPOP_NO_PYTHON);
		throw WINEXCEPT("Python could not be found! Please install Python and restart the IPOP Service");
	}
	if (ERROR_SUCCESS == RegQueryValueEx(regkey, NULL, 0, &type, buf.get(), &len)){
		mPythonPath.assign(path);
	}
	else
	{
		EventLogger.LogEvent(IPOP_NO_PYTHON);
		throw WINEXCEPT("Python could not be found! Please install Python and restart the IPOP Service");
	}

	return;

}

void 
ServiceConfig::QueryIPopPath()
{
	unique_ptr<wchar_t[]> buf = make_unique<wchar_t[]>(sizeof(wchar_t)*MAX_PATH);
	GetModuleFileName(NULL, buf.get(), MAX_PATH);
	wstring path(buf.get());
	size_t pos = path.rfind(L"\\");
	mIPopPath = path.substr(0, pos);

	return;
}

}
}