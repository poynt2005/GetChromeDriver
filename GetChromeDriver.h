#ifndef __GET_CHROME_DRIVER_H__
#define __GET_CHROME_DRIVER_H__

#ifdef BUILDGCDAPI
#define GCDAPI __declspec(dllexport)
#else
#define GCDAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    GCDAPI int GetChromeVersion(char version[30]);
    GCDAPI int DownloadChromeDriver(const char *versionString, const char *driverPath);
    GCDAPI const char *GetLastErrorString();

#ifdef __cplusplus
}
#endif

#endif