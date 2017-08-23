/*******************************************************************
*  Copyright(c) 2000-2013 Company Name
*  All rights reserved.
*
*  文件名称:公共模块定义
*  简要描述:常用公共函数定义
*
*  创建日期:2017-07-26
*  作者:zlb
*  说明:定义常用函数API
*
******************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <libloaderapi.h>
#include <vector>
#include <map>
#include <shlwapi.h>
#include <dos.h>

#pragma comment(lib, "shlwapi.lib")

/*
*	define common function
*/

using namespace std;
/*
*	define Code Page Id
*/
enum CodePageID : unsigned int
{
	ANSI = CP_ACP,	// ANSI
	OEM = CP_OEMCP,	// OEM(依存)
	MAC = CP_MACCP,	// MAC
	UTF7 = CP_UTF7,	// UTF-7
	UTF8 = CP_UTF8	// UTF-8
};
/* 接口声明 start */

/*文件路径获取接口*/
std::string GetModulePathA();//get module path(char)
std::wstring GetModulePathW();//get module path(wchar_t)

std::string GetModuleFilePathA();//get module file path(char)
std::wstring GetModuleFilePathW();//get module file path(wchar_t)

std::string GetModuleFileNameA();//get module file name(char)
std::wstring GetModuleFileNameW();//get module file name(wchar_t)

/*字符串编码及转换接口*/
std::wstring String2WString(const std::string& refSrc, unsigned int codePage = CodePageID::ANSI);//string to wstring
std::string WString2String(const std::wstring& refSrc, unsigned int codePage = CodePageID::OEM);//wstring to string

std::string String2UTF8(const std::string& src);//string to utf-8
std::string UTF82String(const std::string& src);//utf-8 to string

std::string UTF16to8(const wchar_t * in);//utf-16 to utf-8
std::wstring UTF8to16(const char * in);//utf-8 to utf-16

std::string GetFormatDateTime();//get format date time string
std::string FormatStr(const char *format, ...);//format string

std::vector<std::string> GetFolderFilesA(std::string path, std::string ext = "*.*", bool bIsFullPath = true);//get files of folder(char)
std::vector<std::wstring> GetFolderFilesW(std::wstring path, std::wstring ext = L"*.*", bool bIsFullPath = true);//get files of folder(wchar_t)

/* 接口声明 end */

/*
*	define struct path info
*/
typedef struct _PATH_INFO_STRU_A
{
	char path[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_PATH_INFO_STRU_A()
	{
		memset(path, 0, _MAX_PATH);
		memset(drive, 0, _MAX_DRIVE);
		memset(dir, 0, _MAX_DIR);
		memset(fname, 0, _MAX_FNAME);
		memset(ext, 0, _MAX_EXT);
	}
}PATH_INFO_STRU_A, *PPATH_INFO_STRU_A;

typedef struct _PATH_INFO_STRU_W
{
	wchar_t path[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	_PATH_INFO_STRU_W()
	{
		wmemset(path, 0, _MAX_PATH);
		wmemset(drive, 0, _MAX_DRIVE);
		wmemset(dir, 0, _MAX_DIR);
		wmemset(fname, 0, _MAX_FNAME);
		wmemset(ext, 0, _MAX_EXT);
	}
}PATH_INFO_STRU_W, *P_PATH_INFO_STRU_W;
/**
*  功能描述:获取路径信息
*  @param [out] PATH_INFO_STRU_A
*
*  @return void
*/
void GetModuleInfoA(PATH_INFO_STRU_A &pathInfo)
{
	::GetModuleFileNameA(NULL, pathInfo.path, _MAX_PATH);
	_splitpath_s(pathInfo.path, pathInfo.drive, pathInfo.dir, pathInfo.fname, pathInfo.ext);
}
/**
*  功能描述:获取路径信息
*  @param [out] PATH_INFO_STRU_W
*
*  @return void
*/
void GetModuleInfoW(PATH_INFO_STRU_W &pathInfo)
{
	::GetModuleFileNameW(NULL, pathInfo.path, _MAX_PATH);
	_wsplitpath_s(pathInfo.path, pathInfo.drive, pathInfo.dir, pathInfo.fname, pathInfo.ext);
}
/**
*  功能描述:获取程序根目录
*  @param void
*
*  @return std::string
*/
std::string GetModulePathA()
{
	PATH_INFO_STRU_A pathInfo;
	GetModuleInfoA(pathInfo);

	sprintf_s(pathInfo.path, _MAX_PATH, "%s%s", pathInfo.drive, pathInfo.dir);

	return pathInfo.path;
}
/**
*  功能描述:获取程序根目录
*  @param void
*
*  @return std::wstring
*/
std::wstring GetModulePathW()
{
	PATH_INFO_STRU_W pathInfo;
	GetModuleInfoW(pathInfo);

	wsprintf(pathInfo.path, L"%s%s", pathInfo.drive, pathInfo.dir);

	return pathInfo.path;
}
/**
*  功能描述:获取程序文件路径
*  @param void
*
*  @return std::string
*/
std::string GetModuleFilePathA()
{
	PATH_INFO_STRU_A pathInfo;
	GetModuleInfoA(pathInfo);

	return pathInfo.path;
}
/**
*  功能描述:获取程序文件路径
*  @param void
*
*  @return std::wstring
*/
std::wstring GetModuleFilePathW()
{
	PATH_INFO_STRU_W pathInfo;
	GetModuleInfoW(pathInfo);

	return pathInfo.path;
}
/**
*  功能描述:获取程序文件名
*  @param void
*
*  @return std::string
*/
std::string GetModuleFileNameA()
{
	PATH_INFO_STRU_A pathInfo;
	GetModuleInfoA(pathInfo);

	sprintf_s(pathInfo.path, _MAX_PATH, "%s%s", pathInfo.fname, pathInfo.ext);

	return pathInfo.path;
}
/**
*  功能描述:获取程序文件名
*  @param void
*
*  @return std::wstring
*/
std::wstring GetModuleFileNameW()
{
	PATH_INFO_STRU_W pathInfo;
	GetModuleInfoW(pathInfo);

	wsprintf(pathInfo.path, L"%s%s", pathInfo.fname, pathInfo.ext);

	return pathInfo.path;
}
/**
*  功能描述:string to wstring
*  @param param1 [in]const std::string&
*  @param param2 [in]CodePageID
*
*  @return std::wstring
*/
std::wstring String2WString(const std::string& refSrc, unsigned int codePage /*= CodePageID::ANSI*/)
{
	std::wstring strRet;
	if (0 == refSrc.size()) return strRet;

	std::vector<wchar_t> buffer(MultiByteToWideChar(codePage, 0, refSrc.c_str(), -1, nullptr, 0) - 1);
	if (buffer.size())
	{
		MultiByteToWideChar(codePage, 0, refSrc.c_str(), -1, &buffer.front(), buffer.size());
		strRet = std::wstring(buffer.begin(), buffer.end());
	}

	return strRet;
}
/**
*  功能描述:wstring to string
*  @param param1 [in]const std::wstring&
*  @param param2 [in]CodePageID
*
*  @return std::string
*/
std::string WString2String(const std::wstring& refSrc, unsigned int codePage /*= CodePageID::OEM*/)
{
	std::string strRet;
	if (0 == refSrc.size()) return strRet;

	std::vector<char> buffer(WideCharToMultiByte(codePage, 0, refSrc.c_str(), -1, nullptr, 0, nullptr, nullptr) - 1);
	if (buffer.size())
	{
		WideCharToMultiByte(codePage, 0, refSrc.c_str(), -1, &buffer.front(), buffer.size(), nullptr, nullptr);
		strRet = std::string(buffer.begin(), buffer.end());
	}

	return strRet;
}
/**
*  功能描述:string to utf-8
*  @param param1 [in]const std::string&
*
*  @return std::string
*/
std::string String2UTF8(const std::string& src)
{
	return WString2String(String2WString(src), CodePageID::UTF8);
}
/**
*  功能描述:utf-8 to string
*  @param param1 [in]const std::string&
*
*  @return std::string
*/
std::string UTF82String(const std::string& src)
{
	return WString2String(String2WString(src, UTF8));
}
/**
*  功能描述:UTF16 to UTF8
*  @param param1 [in]const wchar_t *
*
*  @return std::string
*/
std::string UTF16to8(const wchar_t * in)
{
	std::string out;
	unsigned int codepoint = 0;
	for (in; *in != 0; ++in)
	{
		if (*in >= 0xd800 && *in <= 0xdbff)
			codepoint = ((*in - 0xd800) << 10) + 0x10000;
		else
		{
			if (*in >= 0xdc00 && *in <= 0xdfff)
				codepoint |= *in - 0xdc00;
			else
				codepoint = *in;

			if (codepoint <= 0x7f)
				out.append(1, static_cast<char>(codepoint));
			else if (codepoint <= 0x7ff)
			{
				out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
				out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
			}
			else if (codepoint <= 0xffff)
			{
				out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
				out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
				out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
			}
			else
			{
				out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
				out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
				out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
				out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
			}
			codepoint = 0;
		}
	}
	return out;
}
/**
*  功能描述:UTF8 to UTF16
*  @param param1 [in]const char *
*
*  @return std::wstring
*/
std::wstring UTF8to16(const char * in)
{
	std::wstring out;
	unsigned int codepoint = 0;
	int following = 0;
	for (in; *in != 0; ++in)
	{
		unsigned char ch = *in;
		if (ch <= 0x7f)
		{
			codepoint = ch;
			following = 0;
		}
		else if (ch <= 0xbf)
		{
			if (following > 0)
			{
				codepoint = (codepoint << 6) | (ch & 0x3f);
				--following;
			}
		}
		else if (ch <= 0xdf)
		{
			codepoint = ch & 0x1f;
			following = 1;
		}
		else if (ch <= 0xef)
		{
			codepoint = ch & 0x0f;
			following = 2;
		}
		else
		{
			codepoint = ch & 0x07;
			following = 3;
		}
		if (following == 0)
		{
			if (codepoint > 0xffff)
			{
				out.append(1, static_cast<wchar_t>(0xd800 + (codepoint >> 10)));
				out.append(1, static_cast<wchar_t>(0xdc00 + (codepoint & 0x03ff)));
			}
			else
				out.append(1, static_cast<wchar_t>(codepoint));
			codepoint = 0;
		}
	}
	return out;
}
/**
*  功能描述:get format date time string
*  @param void
*
*  @return std::string
*/
std::string GetFormatDateTime()
{
	SYSTEMTIME now;
	GetLocalTime(&now);
	char szTime[32] = { 0 };

	sprintf_s(szTime, 32, "%04d-%02d-%02d %02d:%02d:%02d", now.wYear, now.wMonth, now.wDay,
		now.wHour, now.wMinute, now.wSecond);

	return szTime;
}
/**
*  功能描述:format string
*  @param const char *, ...
*
*  @return std::string
*/
std::string FormatStr(const char *format, ...)
{
	char szTmp[MAX_PATH] = { 0 };

	if (NULL == format)
		return szTmp;

	va_list st;
	va_start(st, format);
	vsnprintf_s(szTmp, MAX_PATH, format, st);
	va_end(st);

	return szTmp;
}
/**
*  功能描述:获取文件夹下指定类型文件
*  @param param1 std::string path
*  @param param2 std::string exe = "*.*"默认所有文件类型
*  @param param3 bool bIsFullPath = true 默认全路径
*
*  @return std::vector<std::string>
*/
std::vector<std::string> GetFolderFilesA(std::string path, std::string ext /*= "*.*"*/, bool bIsFullPath /*= true*/)
{
	std::vector<std::string> vecFile;

	if (FALSE == PathFileExistsA(path.c_str()))
	{
		return vecFile;
	}

	WIN32_FIND_DATAA findFileData;
	memset(&findFileData, 0, sizeof(WIN32_FIND_DATAA));

	size_t pos = path.find_last_of("\\/");
	if ((path.size() - 1) != pos)
	{
		path.append("\\");
	}

	std::string strTmp = path;

	path.append(ext); //这里一定要指明通配符，不然不会读取所有文件和目录

	HANDLE hFind = FindFirstFileA(path.c_str(), &findFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return vecFile;
	}

	do
	{
		//判断是否子目录
		if (findFileData.dwFileAttributes == _A_SUBDIR)
		{
			//TO DO：可递归
			continue;
		}

		if (bIsFullPath)
		{
			vecFile.push_back(strTmp + findFileData.cFileName);
		}
		else
		{
			vecFile.push_back(findFileData.cFileName);
		}

		memset(&findFileData, 0, sizeof(WIN32_FIND_DATAA));

	} while (FindNextFileA(hFind, &findFileData));

	FindClose(hFind);

	return vecFile;
}
/**
*  功能描述:获取文件夹下指定类型文件
*  @param param1 std::wstring path
*  @param param2 std::wstring exe = "*.*"默认所有文件类型
*  @param param3 bool bIsFullPath = true 默认全路径
*
*  @return std::vector<std::wstring>
*/
std::vector<std::wstring> GetFolderFilesW(std::wstring path, std::wstring ext /*= L"*.*"*/, bool bIsFullPath /*= true*/)
{
	std::vector<std::wstring> vecFile;

	if (FALSE == PathFileExistsW(path.c_str()))
	{
		return vecFile;
	}

	WIN32_FIND_DATAW findFileData;
	memset(&findFileData, 0, sizeof(WIN32_FIND_DATAW));

	size_t pos = path.find_last_of(L"\\/");
	if ((path.size() - 1) != pos)
	{
		path.append(L"\\");
	}

	std::wstring strTmp = path;

	path.append(ext); //这里一定要指明通配符，不然不会读取所有文件和目录

	HANDLE hFind = FindFirstFileW(path.c_str(), &findFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return vecFile;
	}

	do
	{
		//判断是否子目录
		if (findFileData.dwFileAttributes == _A_SUBDIR)
		{
			//TO DO：可递归
			continue;
		}

		if (bIsFullPath)
		{
			vecFile.push_back(strTmp + findFileData.cFileName);
		}
		else
		{
			vecFile.push_back(findFileData.cFileName);
		}

		memset(&findFileData, 0, sizeof(WIN32_FIND_DATAW));

	} while (FindNextFileW(hFind, &findFileData));

	FindClose(hFind);

	return vecFile;
}

#endif//__COMMON_H__