#include "IniFile.h"
#include <assert.h>

CIniFile::CIniFile()
{

}

CIniFile::CIniFile(LPCTSTR szFileName) : m_strFileName(szFileName)
{

}

void CIniFile::SetFileName(LPCTSTR szFileName)
{
    m_strFileName = szFileName;
}

BOOL CIniFile::SetUint(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, UINT nKeyValue)
{
    assert(NULL != lpszSectionName && NULL != lpszKeyName);
    return ::WritePrivateProfileString(
        lpszSectionName,
        lpszKeyName,
        To_CString(nKeyValue).c_str(),
        m_strFileName.c_str()
        );
}

BOOL CIniFile::SetInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int nKeyValue)
{
	assert(NULL != lpszSectionName && NULL != lpszKeyName);
	return ::WritePrivateProfileString(
		lpszSectionName,
		lpszKeyName,
		To_CString(nKeyValue).c_str(),
		m_strFileName.c_str()
		);
}

BOOL CIniFile::SetString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszKeyValue)
{
    assert(NULL != lpszSectionName && NULL != lpszKeyName && NULL != lpszKeyValue);
    return ::WritePrivateProfileString(
        lpszSectionName,
        lpszKeyName,
        lpszKeyValue,
        m_strFileName.c_str()
        );
}

UINT CIniFile::GetUint(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int defaultVal /*= 0 */)
{
    assert(NULL != lpszSectionName && NULL != lpszKeyName);
    return ::GetPrivateProfileInt(
        lpszSectionName,
        lpszKeyName,
        defaultVal,
        m_strFileName.c_str()
        );
}

int CIniFile::GetInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int defaultVal /*= 0 */)
{
	assert(NULL != lpszSectionName && NULL != lpszKeyName);
	return ::GetPrivateProfileInt(
		lpszSectionName,
		lpszKeyName,
		defaultVal,
		m_strFileName.c_str()
		);
}

DWORD CIniFile::GetString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault, C_String &strVal)
{
    assert(NULL != lpszSectionName && NULL != lpszKeyName);
    TCHAR szbuff[MAX_STR_BLOCKREASON] = { 0 };

    DWORD dwRet = ::GetPrivateProfileString(
        lpszSectionName,
        lpszKeyName,
        lpszDefault,
        szbuff,
        MAX_STR_BLOCKREASON,
        m_strFileName.c_str()
        );

    strVal = szbuff;

    return dwRet;
}

BOOL CIniFile::DeleteSection(LPCTSTR lpszSectionName)
{
    return ::WritePrivateProfileSection(
        lpszSectionName,
        NULL,
        m_strFileName.c_str()
        );
}

BOOL CIniFile::DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)
{
    return ::WritePrivateProfileString(
        lpszSectionName,
        lpszKeyName,
        NULL,
        m_strFileName.c_str()
        );
}
