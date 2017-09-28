#include "IniFile.h"
/************************************************************************/
/* INI file Opt use Windows Api                                         */
/************************************************************************/

int main()
{
    CIniFile iniFile(L"./iniFile.ini");

    iniFile.SetUint(L"UINT", L"uint", UINT_MAX);
    UINT uInt = iniFile.GetUint(L"UINT", L"uint");
    printf("SetUint = %u\n", UINT_MAX);
    printf("GetUint = %u\n", uInt);

    iniFile.SetInt(L"INT", L"int", -INT_MAX);
    int i = iniFile.GetInt(L"INT", L"int");
    printf("SetUint = %d\n", -INT_MAX);
    printf("GetUint = %d\n", i);

    C_String ss(L"hello world!");
    iniFile.SetString(L"STRING", L"string", ss.c_str());
    wprintf(L"SetString = %s\n", ss.c_str());

    C_String retStr;
    iniFile.GetString(L"STRING", L"string", L"", retStr);
    wprintf(L"GetString = %s\n", retStr.c_str());

    iniFile.DeleteSection(L"STRING");

    iniFile.DeleteKey(L"UINT", L"uint");

    iniFile.SetUint(L"UINT", L"uint", UINT_MAX);
    uInt = iniFile.GetUint(L"UINT", L"uint");
    printf("SetUint = %u\n", UINT_MAX);
    printf("GetUint = %u\n", uInt);
    return 0;
}