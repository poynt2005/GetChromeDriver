#include <Windows.h>
#include <filesystem>
#include <string>
#include <string.h>
#include <time.h>

#include <sstream>
#include "GetChromeDriver.h"

//
#include <iostream>

std::string lastErrorString;

using LPGoFuncFindUrlByVersionString = char *(*)(char *);
using LPGoFuncGetChromeVersion = char *(*)();
using LPGoFuncGetLastErrorString = char *(*)();
using LPGoFuncFreeGoBuffer = void (*)(char **);
using LPGoFuncUnzipDriver = int (*)(char *, char *);

using GoFuncContext = struct goFuncContext
{
    LPGoFuncFindUrlByVersionString GoFuncFindUrlByVersionString = nullptr;
    LPGoFuncGetChromeVersion GoFuncGetChromeVersion = nullptr;
    LPGoFuncGetLastErrorString GoFuncGetLastErrorString = nullptr;
    LPGoFuncFreeGoBuffer GoFuncFreeGoBuffer = nullptr;
    LPGoFuncUnzipDriver GoFuncUnzipDriver = nullptr;
};

static GoFuncContext GoCtx;
static HMODULE library = nullptr;

bool InitGoFuncContext()
{
    if (library != nullptr)
    {
        return true;
    }

    library = LoadLibrary("GoContext.dll");
    if (library == nullptr)
    {
        lastErrorString = "LoadDllFailed";
        return false;
    }

    GoCtx.GoFuncFindUrlByVersionString = (LPGoFuncFindUrlByVersionString)GetProcAddress(library, "FindUrlByVersionString");
    GoCtx.GoFuncGetChromeVersion = (LPGoFuncGetChromeVersion)GetProcAddress(library, "GetChromeVersion");
    GoCtx.GoFuncGetLastErrorString = (LPGoFuncGetLastErrorString)GetProcAddress(library, "GetLastErrorString");
    GoCtx.GoFuncFreeGoBuffer = (LPGoFuncFreeGoBuffer)GetProcAddress(library, "FreeGoBuffer");
    GoCtx.GoFuncUnzipDriver = (LPGoFuncUnzipDriver)GetProcAddress(library, "UnzipDriver");

    if ((GoCtx.GoFuncFindUrlByVersionString == nullptr) || (GoCtx.GoFuncGetChromeVersion == nullptr) || (GoCtx.GoFuncGetLastErrorString == nullptr) || (GoCtx.GoFuncFreeGoBuffer == nullptr) || (GoCtx.GoFuncUnzipDriver == nullptr))
    {
        lastErrorString = "LoadFuncFromDllFailed";
        return false;
    }

    return true;
}

std::string timestr()
{
    std::stringstream strm;
    strm << time(0);
    return strm.str();
}

int GetChromeVersion(char version[30])
{
    lastErrorString = "";

    if (!InitGoFuncContext())
    {
        return 0;
    }

    auto chromeVersion = GoCtx.GoFuncGetChromeVersion();

    if (chromeVersion == nullptr)
    {
        auto goLEStr = GoCtx.GoFuncGetLastErrorString();
        lastErrorString = std::string(goLEStr);
        GoCtx.GoFuncFreeGoBuffer(&goLEStr);
        return 0;
    }

    memset(version, '\0', 30);
    memcpy(version, chromeVersion, strlen(chromeVersion));

    GoCtx.GoFuncFreeGoBuffer(&chromeVersion);

    return 1;
}

int DownloadChromeDriver(const char *versionString, const char *driverPath)
{
    lastErrorString = "";

    if (!InitGoFuncContext())
    {
        return 0;
    }

    auto url = GoCtx.GoFuncFindUrlByVersionString(const_cast<char *>(versionString));

    if (url == nullptr)
    {
        auto goLEStr = GoCtx.GoFuncGetLastErrorString();
        lastErrorString = std::string(goLEStr);
        GoCtx.GoFuncFreeGoBuffer(&goLEStr);
        return 0;
    }

    auto targetZipPath = (std::filesystem::temp_directory_path() / (timestr() + ".zip")).string();

    auto hr = URLDownloadToFile(
        nullptr,
        url,
        targetZipPath.c_str(),
        0,
        nullptr
        //
    );

    GoCtx.GoFuncFreeGoBuffer(&url);

    if (hr != S_OK)
    {
        lastErrorString = "DownloadFromURLFailed";

        if (std::filesystem::exists(targetZipPath))
        {
            std::filesystem::remove(targetZipPath);
        }
    }
    else
    {

        if (!GoCtx.GoFuncUnzipDriver(const_cast<char *>(targetZipPath.c_str()), const_cast<char *>(driverPath)))
        {
            auto goLEStr = GoCtx.GoFuncGetLastErrorString();
            lastErrorString = std::string(goLEStr);
            GoCtx.GoFuncFreeGoBuffer(&goLEStr);
            return 0;
        }

        return 1;
    }

    return 0;
}

const char *GetLastErrorString()
{
    if (lastErrorString.empty() || !lastErrorString.length())
    {
        return nullptr;
    }
    return lastErrorString.c_str();
}
