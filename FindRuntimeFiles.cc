#include <Windows.h>
#include <memory>
#include <stddef.h>

extern HMODULE hModule;

std::unique_ptr<char[]> FindPyRuntimeFiles(const unsigned int resourceId, size_t &bufferSize)
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
}