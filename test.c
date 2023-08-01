#include <stdio.h>
#include "GetChromeDriver.h"

int main()
{
    char version[30] = {'\0'};
    if (!GetChromeVersion(version))
    {
        printf("%s\n", GetLastErrorString());
        return 1;
    }

    printf("Chrome Version is: %s\n", version);

    if (!DownloadChromeDriver(version, "./driver.exe"))
    {
        printf("%s\n", GetLastErrorString());

        const char *altVersion = "114.0.5735.90";
        if (!DownloadChromeDriver(altVersion, "./driver.exe"))
        {
            printf("%s\n", GetLastErrorString());
            printf("Try alt version: %s failed\n", altVersion);
        }
        else
        {
            printf("Try alt version: %s success\n", altVersion);
            return 0;
        }

        return 1;
    }
    return 0;
}