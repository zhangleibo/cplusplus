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
typedef std::wstring C_String;
#define To_CString(val) std::to_wstring(val)
#else
typedef std::string C_String;
#define To_CString(val) std::to_string(val)
#endif

class CIniFile
{
public:
    CIniFile();
    CIniFile(LPCTSTR szFileName);

    void SetFileName(LPCTSTR szFileName);

    //Operations
    BOOL SetUint(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName,
        UINT nKeyValue
        );
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
    UINT GetUint(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName,
        int defaultVal = 0
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
        C_String &strVal
        );
    BOOL DeleteSection(
        LPCTSTR lpszSectionName
        );
    BOOL DeleteKey(
        LPCTSTR lpszSectionName,
        LPCTSTR lpszKeyName
        );
private:
    C_String m_strFileName;
};