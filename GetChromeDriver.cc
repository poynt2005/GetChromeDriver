#include <Python.h>
#include <filesystem>
#include <Windows.h>
#include <PsApi.h>
#include <string.h>
#include <string>
#include <memory>
#include <stdio.h>
#include <optional>
#include <algorithm>
#include "Zipper2/Zipper.h"
#include "Resource.h"
#include "GetChromeDriver.h"
//
// #include <iostream>

#define MAX_MODULE_NUM 2048

#define LOWER(STR) [&]() -> decltype(auto) {                                                      \
    std::string s(STR);                                                                           \
    std::transform(s.begin(), s.end(), s.begin(), [](const auto &c) { return std::tolower(c); }); \
    return s;                                                                                     \
}();

extern std::unique_ptr<char[]> FindPyRuntimeFiles(const unsigned int resourceId, size_t &bufferSize);

static PyObject *pyGetChromeDriverModule = nullptr;
static bool isPyRuntimeInit = false;
static std::string lastErrorString = "";
static std::string tempPath(256, '\0');

// extern static con't put together
HMODULE hModule = nullptr;

static inline std::optional<std::wstring> ToWString(const std::string &strInput)
{
    auto size = MultiByteToWideChar(CP_UTF8, 0, strInput.c_str(), strInput.length(), nullptr, 0);

    if (!size)
    {
        return std::nullopt;
    }

    auto wchars = std::make_unique<wchar_t[]>(size + 1);
    ZeroMemory(wchars.get(), size + 1);

    auto result = MultiByteToWideChar(CP_UTF8, 0, strInput.c_str(), strInput.length(), wchars.get(), size);

    if (!result)
    {
        return std::nullopt;
    }

    return std::wstring(wchars.get());
}

static inline std::optional<std::string> PyStringObjAsStdString(PyObject *pyUnicodeObj)
{
    if (!PyUnicode_Check(pyUnicodeObj))
    {
        return std::nullopt;
    }

    auto pyByteObj = PyUnicode_AsEncodedString(pyUnicodeObj, "utf-8", "ignore");

    if (pyByteObj == nullptr)
    {
        return std::nullopt;
    }

    auto chars = PyBytes_AsString(pyByteObj);

    if (chars == nullptr)
    {
        Py_XDECREF(pyByteObj);
        return std::nullopt;
    }

    auto str = std::string(chars);
    Py_XDECREF(pyByteObj);

    return str;
}

// return whether job is done
static bool TogglePyRuntime(bool toInit = true)
{
    // lastErrorString.resize(0);
    // lastErrorString.shrink_to_fit();

    if (!isPyRuntimeInit && toInit)
    {
        if (tempPath[0] == '\0')
        {

            if (!GetTempPath(256, tempPath.data()))
            {
                lastErrorString = "CannotGetTempPath";
                return false;
            }

            // remove null characters
            tempPath = tempPath.substr(0, tempPath.find_first_of('\0'));
            if (tempPath[tempPath.length() - 1] == '\\')
            {
                tempPath = tempPath.substr(0, tempPath.length() - 1);
            }
        }

        auto runtimeDir = (std::filesystem::path(tempPath) / "GetChromeDriverRuntime").string();

        if (!std::filesystem::is_directory(runtimeDir))
        {
            size_t pyBufferSize = 0;
            auto spPyRuntimeBuffer = FindPyRuntimeFiles(ID_PyRuntime, pyBufferSize);

            if (spPyRuntimeBuffer != nullptr && pyBufferSize)
            {
                auto runtimeZipBallName = runtimeDir + ".zip";

                auto fp = fopen(runtimeZipBallName.c_str(), "wb");
                fwrite(spPyRuntimeBuffer.get(), pyBufferSize, sizeof(char), fp);
                fclose(fp);

                Zipper zipper;
                auto isZipUnpacked = zipper.UnPackArchive(runtimeZipBallName, runtimeDir);

                if (!isZipUnpacked)
                {
                    lastErrorString = "UnPackPyRuntimeZipFailed";
                    return false;
                }

                DeleteFile(runtimeZipBallName.c_str());
            }
            else
            {
                lastErrorString = "GetPyRuntimeResourceFileFailed";
                return false;
            }
        }

        auto wstrPythonHomeZip = (std::filesystem::path(runtimeDir) / "python310.zip").wstring();

        Py_SetPythonHome(wstrPythonHomeZip.c_str());
        Py_SetPath(wstrPythonHomeZip.c_str());

        Py_Initialize();
        PyErr_Print();

        auto pythonAppendScript = "import sys;sys.path.insert(0, '');sys.path.append(r'" + runtimeDir + "');";
        PyRun_SimpleString(pythonAppendScript.c_str());

        size_t pyModuleCodeRawBufferSize = 0;
        auto spModuleRawBuffer = FindPyRuntimeFiles(ID_PyGetChromeDriverCode, pyModuleCodeRawBufferSize);

        if (spModuleRawBuffer == nullptr)
        {
            lastErrorString = "GetPyCodeResourceFileFailed";
            return false;
        }

        // Dispose leading 16 bytes data
        auto spModuleBuffer = std::make_unique<char[]>(pyModuleCodeRawBufferSize - 16);
        memcpy(spModuleBuffer.get(), spModuleRawBuffer.get() + 16, pyModuleCodeRawBufferSize - 16);

        auto pyCodeBlob = PyBytes_FromStringAndSize(spModuleBuffer.get(), pyModuleCodeRawBufferSize - 16);

        auto pyMarshalModule = PyImport_ImportModule("marshal");
        auto pyCodeObject = PyObject_CallMethod(pyMarshalModule, "loads", "O", pyCodeBlob);

        pyGetChromeDriverModule = PyImport_ExecCodeModule("GetChromeDriver", pyCodeObject);

        Py_XDECREF(pyCodeObject);
        Py_XDECREF(pyMarshalModule);

        isPyRuntimeInit = true;
    }

    if (isPyRuntimeInit && !toInit)
    {
        Py_XDECREF(pyGetChromeDriverModule);
        pyGetChromeDriverModule = nullptr;

        Py_Finalize();
        isPyRuntimeInit = false;

        auto pid = GetCurrentProcessId();
        auto handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

        if (handle != nullptr)
        {
            HMODULE modules[MAX_MODULE_NUM];
            DWORD needed;
            if (EnumProcessModules(handle, modules, MAX_MODULE_NUM, &needed))
            {
                int moduleCount = needed / sizeof(HMODULE);
                for (int i = 0; i < moduleCount; i++)
                {
                    char moduleName[MAX_PATH];
                    if (GetModuleFileName(modules[i], moduleName, MAX_PATH))
                    {
                        auto modulePath = std::filesystem::path(moduleName);
                        if (modulePath.extension().string() == ".pyd")
                        {
                            FreeLibrary(modules[i]);
                        }
                    }
                }
            }
        }
    }

    return true;
}

int GetChromeVersion(char version[30])
{
    lastErrorString.resize(0);
    lastErrorString.shrink_to_fit();

    if (!TogglePyRuntime(true))
    {
        return 0;
    }

    auto pyVersionQueryResult = PyObject_CallMethod(pyGetChromeDriverModule, "get_chrome_version", nullptr);
    if (!PyDict_Check(pyVersionQueryResult))
    {
        lastErrorString = "VersionQueryResultNotAValiedDict";
        Py_XDECREF(pyVersionQueryResult);
        return 0;
    }

    auto pyIsSUccess = PyDict_GetItemString(pyVersionQueryResult, "is_success");

    if (PyObject_IsTrue(pyIsSUccess))
    {
        auto versionString = PyStringObjAsStdString(PyDict_GetItemString(pyVersionQueryResult, "version"));

        if (!versionString.has_value())
        {
            lastErrorString = "ConvertVersionStringFailed";
        }
        else
        {
            memcpy(version, versionString.value().c_str(), versionString.value().length());
        }
    }
    else
    {
        auto errReasonString = PyStringObjAsStdString(PyDict_GetItemString(pyVersionQueryResult, "reason"));
        if (!errReasonString.has_value())
        {
            lastErrorString = "ConvertErrorStringFailed";
        }
        else
        {
            lastErrorString = errReasonString.value();
        }
    }

    Py_XDECREF(pyVersionQueryResult);

    TogglePyRuntime(false);

    if (lastErrorString.empty())
    {
        return 1;
    }

    return 0;
}
int DownloadChromeDriver(const char *versionString, const char *driverPath)
{
    lastErrorString.resize(0);
    lastErrorString.shrink_to_fit();

    if (!TogglePyRuntime(true))
    {
        return 0;
    }

    auto pyGetChromeDriverUrlResult = PyObject_CallMethod(pyGetChromeDriverModule, "get_chrome_driver", "s", versionString);

    if (!PyDict_Check(pyGetChromeDriverUrlResult))
    {
        lastErrorString = "DriverUrlResultNotAValiedDict";
        Py_XDECREF(pyGetChromeDriverUrlResult);
        return 0;
    }

    auto pyIsSUccess = PyDict_GetItemString(pyGetChromeDriverUrlResult, "is_success");

    std::string urlString;
    if (PyObject_IsTrue(pyIsSUccess))
    {

        auto urlStringResult = PyStringObjAsStdString(PyDict_GetItemString(pyGetChromeDriverUrlResult, "url"));

        if (!urlStringResult.has_value())
        {
            lastErrorString = "ConvertUrlStringFailed";
        }
        else
        {
            urlString = urlStringResult.value();
        }
    }
    else
    {
        auto errReasonString = PyStringObjAsStdString(PyDict_GetItemString(pyGetChromeDriverUrlResult, "reason"));

        if (!errReasonString.has_value())
        {
            lastErrorString = "ConvertErrorStringFailed";
        }
        else
        {
            lastErrorString = errReasonString.value();
        }
    }
    Py_XDECREF(pyGetChromeDriverUrlResult);
    TogglePyRuntime(false);

    if (urlString.empty())
    {
        return 0;
    }

    auto getChromeDriverTempDir = (std::filesystem::path(tempPath) / "GetChromeDriverTempDir").string();

    if (!std::filesystem::is_directory(getChromeDriverTempDir))
    {
        std::filesystem::create_directory(getChromeDriverTempDir);
    }

    auto zipballName = (std::filesystem::path(getChromeDriverTempDir) / "driver.zip").string();

    auto hResult = URLDownloadToFile(
        nullptr,
        urlString.c_str(),
        zipballName.c_str(),
        0,
        nullptr
        //
    );

    if (hResult != S_OK)
    {
        lastErrorString = "DownloadDriverZipFailed";
        std::filesystem::remove_all(getChromeDriverTempDir);
        return 0;
    }

    auto driverStorePath = std::filesystem::path(getChromeDriverTempDir) / "driver";
    Zipper zipper;
    if (!zipper.UnPackArchive(zipballName, driverStorePath.string()))
    {
        lastErrorString = "UnzipDriverFailed";
        std::filesystem::remove_all(getChromeDriverTempDir);
        return 0;
    }

    std::string targetDriverFile;
    for (const auto &entry : std::filesystem::directory_iterator(driverStorePath))
    {
        auto extension = LOWER(entry.path().extension().string());

        if (extension == ".exe")
        {
            targetDriverFile = std::filesystem::absolute(entry.path()).string();
            break;
        }
    }

    if (targetDriverFile.empty())
    {
        lastErrorString = "GetDriverExecutableFailed";
        std::filesystem::remove_all(getChromeDriverTempDir);
        return 0;
    }

    std::filesystem::rename(
        targetDriverFile,
        driverPath
        //
    );

    if (!std::filesystem::is_regular_file(driverPath))
    {
        lastErrorString = "MoveDriverExecutableFailed";
        std::filesystem::remove_all(getChromeDriverTempDir);
        return 0;
    }

    std::filesystem::remove_all(getChromeDriverTempDir);
    return 1;
}

const char *GetLastErrorString()
{
    return lastErrorString.c_str();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    hModule = static_cast<HMODULE>(hinstDLL);
    return TRUE;
}