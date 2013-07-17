/*
 unsigned char*
*/

#ifndef BASE_WIN32_H
#define BASE_WIN32_H

#include "detect.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
int W32RegGetKeyData(HKEY hRootKey, char *subKey, char *value, LPBYTE data, DWORD cbData)
{
    HKEY hKey;
    if(RegOpenKeyEx(hRootKey, subKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 0;

    if(RegQueryValueEx(hKey, value, NULL, NULL, data, &cbData) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }

    RegCloseKey(hKey);
    return 1;
}

int W32RegCreateKey(HKEY hRootKey, char *subKey)
{
    HKEY hKey;
    if(RegCreateKey(hRootKey, subKey, &hKey) != ERROR_SUCCESS)
        return 0;

    RegCloseKey(hKey);
    return 1;
}

int W32RegExistsKey(HKEY hRootKey, char *subKey)
{
    HKEY hKey;
    if(RegOpenKeyEx(hRootKey, subKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 0;

    RegCloseKey(hKey);
    return 1;
}

int W32RegSetKeyData(HKEY hRootKey, char *subKey, DWORD dwType, char *value, LPBYTE data, DWORD cbData)
{
    HKEY hKey;
    if(RegCreateKey(hRootKey, subKey, &hKey) != ERROR_SUCCESS)
        return 0;

    if(RegSetValueEx(hKey, value, 0, dwType, data, cbData) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }

    RegCloseKey(hKey);
    return 1;
}
*/
#ifdef __cplusplus
}
#endif

#endif
