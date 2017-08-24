#pragma once
#include <windows.h>
#include <tchar.h>
#include <string>

/*
*@description:Ini File opt use windows api
*@date:2017-8-24 22:01:58
*@author:zlb
*/

#ifdef UNICODE
typedef std::wstring CString;
#define To_CString(val) std::to_wstring(val)
#else
typedef std::string CString;
#define To_CString(val) std::to_string(val)
#endif

class CIniFile
{
public:
    CIniFile();
    CIniFile(LPCTSTR szFileName);

    void SetFileName(LPCTSTR szFileName);

    //Operations
    BOOL SetInt(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName,
        int nKeyValue
        );
    BOOL SetString(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName,
        LPCTSTR lpszKeyValue
        );
    int GetInt(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName,
        int defaultVal = 0
        );
    DWORD GetString(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName,
        LPCTSTR lpszDefault,
        CString &strVal
        );
    BOOL DeleteSection(
        LPCTSTR lpszSectionName
        );
    BOOL DeleteKey(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName
        );
private:
    CString m_strFileName;
};