/**************************************************************************
				WinException 
			-------------------
	description			: A Win32 specialization of the std::exception.
	Usage: Throw WinEXCEPTION("with your error message"). When caugth, ask the exception.what()
	happened to get your error message, source code file and line, and the Windows specific error
	message.
	
***************************************************************************/

#pragma once

#include <sstream>
#include <string>
#include <exception>
#include <cstdlib>
using namespace std;
#include <windows.h>

#define WINEXCEPT(ExtendedErrorInfo) 	WinException(ExtendedErrorInfo, __FILE__, __LINE__)

namespace ipop
{
namespace win
{

		class WinException : virtual public exception{
		private:
			unsigned int	mSourceLine;
			unsigned int	mHostErrorCode;
			const string mSourceFile;
			const string mExtendedErrorInfo;
			string mExcept;

		protected:
			__inline void BuildMsg();

		public:
			__inline WinException();
			__inline WinException(
				const char *const & ExtendedErrorInfo,
				const char *const & SourceFile,
				const unsigned int SourceLine);
			__inline ~WinException();
			__inline const char* what()const;

		};

		WinException::WinException(
			const char *const & ExtendedErrorInfo,
			const char *const & SourceFile,
			const unsigned int SourceLine) :
			exception(ExtendedErrorInfo),
			mSourceFile(SourceFile),
			mExtendedErrorInfo(ExtendedErrorInfo),
			mSourceLine(SourceLine),
			mHostErrorCode(GetLastError())
		{
			BuildMsg();
			return;
		}


		WinException::WinException() :exception(){
			WinException("Exception", __FILE__, __LINE__);
		}

		WinException::~WinException(){
			//delete mSourceFile;
			//delete mHostErrorInfo;
			//delete mExtendedErrorInfo;
		}

		void WinException::BuildMsg(){
			LPSTR MsgBuf = NULL;
			string errcode;
			string srcline;
			string HostErrorInfo;

			DWORD dw = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				mHostErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPSTR)&MsgBuf,
				12,
				NULL);
			if (dw && MsgBuf){
				HostErrorInfo.append(MsgBuf);
				memset(MsgBuf, 0x0, 0XC);//width of an int represented in decimal form + null terminator
				_snprintf_s(static_cast<char*>(MsgBuf), 0xC, 0xB, "%d", mSourceLine);
				srcline = static_cast<char*>(MsgBuf);
				memset(MsgBuf, 0x0, 12);
				_snprintf_s(static_cast<char*>(MsgBuf), 0xC, 0xB, "%d", mHostErrorCode);
				errcode = static_cast<char*>(MsgBuf);

				LocalFree(MsgBuf);
				MsgBuf = NULL;
			}
			mExcept += "(";
			mExcept += mSourceFile;
			mExcept += ":";
			mExcept += srcline;
			mExcept += ")";
			mExcept += '\n';
			mExcept += errcode;
			mExcept += " - ";
			mExcept += HostErrorInfo;
			mExcept += mExtendedErrorInfo;
			mExcept += '\n';
		}

		const char* WinException::what() const{

			return mExcept.c_str();

		}

	}
}
