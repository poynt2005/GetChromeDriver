#include "Zipper.h"
#include <string>
#include <filesystem>
#include <Windows.h>
#include <filesystem>
#include <stddef.h>
#include <stdio.h>
#include <type_traits>
#include "Resource.h"

// #include <iostream>

static std::string tempPath(256, '\0');

static HMODULE hModule;

using pack_t = struct __extract_t
{
    static constexpr char *name = "pack";
};

using unpack_t = struct __unpack_t
{
    static constexpr char *name = "unpack";
};

#define GET_TEMP_PATH(ERROR_RESULT)                                  \
    if (tempPath[0] == '\0')                                         \
    {                                                                \
        if (!GetTempPath(256, tempPath.data()))                      \
        {                                                            \
            ERROR_RESULT = "CannotGetTempPath";                      \
        }                                                            \
        tempPath = tempPath.substr(0, tempPath.find_first_of('\0')); \
        if (tempPath[tempPath.length() - 1] == '\\')                 \
        {                                                            \
            tempPath = tempPath.substr(0, tempPath.length() - 1);    \
        }                                                            \
    }

#define ZIP_RT_DIR (std::filesystem::path(tempPath) / "7zRTTempDir").string()

#define WRITE_RC_FILE(BUFFER, FILENAME, SIZE) [&]() -> void { \
    auto fp = fopen(FILENAME, "wb");                          \
    fwrite(BUFFER, SIZE, sizeof(char), fp);                   \
    fclose(fp);                                               \
}();

#define PACK_CMD(EXE_PATH, SRC_PATH, DST_PATH) ('"' + std::string(EXE_PATH) + '"' + " a " + '"' + DST_PATH + '"' + " " + '"' + SRC_PATH + "\\*" + '"')
#define UNPACK_CMD(EXE_PATH, SRC_PATH, DST_PATH) ('"' + std::string(EXE_PATH) + '"' + " x " + '"' + SRC_PATH + '"' + " -o" + '"' + DST_PATH + '"')

Zipper::Zipper()
{
    isServenZipReady = ExtractSevenZipRuntime();
}

Zipper::~Zipper()
{
    ClearSevenZipRuntime();
}

template <typename T>
bool Zipper::CreateServenZipJob(const std::string &srcPath, const std::string &dstPath) const
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // si.dwFlags = STARTF_USESTDHANDLES;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    std::string cmd;
    if constexpr (std::is_same_v<T, unpack_t>)
    {
        cmd = UNPACK_CMD(exePath, srcPath, dstPath);
    }
    else if constexpr (std::is_same_v<T, pack_t>)
    {
        cmd = PACK_CMD(exePath, srcPath, dstPath);
    }

    if (!CreateProcess(
            nullptr,
            const_cast<char *>(cmd.c_str()),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &si,
            &pi
            //
            ))
    {
        errorCode = "RunSevenCommandFailed";
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    TerminateProcess(pi.hProcess, 300);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

bool Zipper::PackArchive(const std::string &directoryName, const std::string zipballName) const
{

    errorCode.resize(0);
    errorCode.shrink_to_fit();

    if (!isServenZipReady)
    {
        errorCode = "ServerZipRuntimeNotReady";
        return false;
    }

    if (!std::filesystem::is_directory(directoryName))
    {
        errorCode = "TargetFileIsNotDirectory";
        return false;
    }

    auto dirAbsPath = std::filesystem::absolute(directoryName).string();
    auto zipAbsPath = std::filesystem::absolute(zipballName).string();

    auto isJobFinish = CreateServenZipJob<pack_t>(dirAbsPath, zipAbsPath);

    if (!isJobFinish)
    {
        return false;
    }

    if (!std::filesystem::is_regular_file(zipAbsPath))
    {
        errorCode = "CreateZipballFailed";
        return false;
    }

    return true;
}

bool Zipper::UnPackArchive(const std::string zipballName, const std::string &directoryName) const
{
    errorCode.resize(0);
    errorCode.shrink_to_fit();

    if (!isServenZipReady)
    {
        errorCode = "ServerZipRuntimeNotReady";
        return false;
    }

    if (std::filesystem::is_directory(directoryName))
    {
        std::filesystem::remove_all(directoryName);
    }

    auto dirAbsPath = std::filesystem::absolute(directoryName).string();
    auto zipAbsPath = std::filesystem::absolute(zipballName).string();

    auto isJobFinish = CreateServenZipJob<unpack_t>(zipAbsPath, dirAbsPath);

    if (!isJobFinish)
    {
        return false;
    }

    if (!std::filesystem::is_directory(dirAbsPath))
    {
        errorCode = "UnpackZipballFailed";
        return false;
    }

    return true;
}

bool Zipper::ExtractSevenZipRuntime()
{
    errorCode.resize(0);
    errorCode.shrink_to_fit();

    GET_TEMP_PATH(errorCode)
    auto serverZipDir = ZIP_RT_DIR;

    if (!std::filesystem::exists(serverZipDir))
    {
        std::filesystem::create_directory(serverZipDir);
    }

    auto Find7zRT = [](const unsigned int resourceId, size_t &bufferSize) -> std::unique_ptr<char[]>
    {
        auto hRsrc = FindResource(
            hModule,
            MAKEINTRESOURCE(resourceId),
            "FILE"
            //
        );

        if (hRsrc == nullptr)
        {
            return nullptr;
        }

        auto hGlobal = LoadResource(
            hModule,
            hRsrc
            //
        );

        if (hGlobal == nullptr)
        {
            return nullptr;
        }

        bufferSize = SizeofResource(
            hModule,
            hRsrc
            //
        );

        if (!bufferSize)
        {
            return nullptr;
        }

        auto pBuffer = LockResource(hGlobal);

        if (pBuffer == nullptr)
        {
            return nullptr;
        }

        auto spBuffer = std::make_unique<char[]>(bufferSize);
        RtlCopyMemory(
            spBuffer.get(),
            reinterpret_cast<char *>(pBuffer),
            bufferSize);
        return spBuffer;
    };

    size_t dllBufferSize = 0;
    size_t exeBufferSize = 0;

    auto dllBuffer = Find7zRT(ID_7Z_DLL, dllBufferSize);
    auto exeBuffer = Find7zRT(ID_7Z_EXE, exeBufferSize);

    if (dllBuffer == nullptr || exeBuffer == nullptr)
    {
        errorCode = "CannotInstall7zRuntime";
        return false;
    }

    const auto dllPath = std::filesystem::absolute(std::filesystem::path(serverZipDir) / "7z.dll").string();
    exePath = std::filesystem::absolute(std::filesystem::path(serverZipDir) / "7z.exe").string();

    WRITE_RC_FILE(dllBuffer.get(), dllPath.c_str(), dllBufferSize)
    WRITE_RC_FILE(exeBuffer.get(), exePath.c_str(), exeBufferSize)

    if (!std::filesystem::exists(dllPath) || !std::filesystem::exists(exePath))
    {
        errorCode = "CannotWrite7zRuntime";
        return false;
    }

    return true;
}

void Zipper::ClearSevenZipRuntime() const
{
    errorCode.resize(0);
    errorCode.shrink_to_fit();

    GET_TEMP_PATH(errorCode)

    auto serverZipDir = ZIP_RT_DIR;

    if (std::filesystem::is_directory(serverZipDir))
    {
        std::filesystem::remove_all(serverZipDir);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    hModule = static_cast<HMODULE>(hinstDLL);
    return TRUE;
}