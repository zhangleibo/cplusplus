#pragma once

#include <windows.h>
#include "zip/unzip.h"
#include "zip/zip.h"
#include <xstring>
#include <string>

class CUnzipFile
{
public:
	void UnzipFile(std::wstring unzipFileName, std::string strPassWord = "", std::wstring subDir = L"")
	{
		if (strPassWord.empty())
		{
			m_hzip = OpenZip(unzipFileName.c_str(), 0);
		}
		else
		{
			m_hzip = OpenZip(unzipFileName.c_str(), strPassWord.c_str());
		}

		if (!subDir.empty()) SetUnzipBaseDir(m_hzip, subDir.c_str());

		ZIPENTRY ze;
		GetZipItem(m_hzip, -1, &ze);
		int numitems = ze.index;

		for (int i = 0; i < numitems; ++i)
		{
			GetZipItem(m_hzip, i, &ze);
			UnzipItem(m_hzip, i, ze.name);
		}

		CloseZip(m_hzip);
	}
private:
	HZIP m_hzip;
};

class CZipFile
{
public:
	void ZipCreateFile(std::wstring zipFileName, std::string strPassWord = "")
	{
		if (strPassWord.empty())
		{
			m_hzip = CreateZip(zipFileName.c_str(), 0);
		}
		else
		{
			m_hzip = CreateZip(zipFileName.c_str(), strPassWord.c_str());
		}
	}

	bool ZipAddFile(std::wstring &fileName, std::wstring &filePath)
	{
		return 0 == ZipAdd(m_hzip, fileName.c_str(), filePath.c_str());
	}

	void ZipCloseFile()
	{
		CloseZip(m_hzip);
	}

private:
	HZIP m_hzip;
};