#include <type_traits>

#include "Zipper.h"

using pack_t = struct __extract_t
{
    static constexpr char *name = "pack";
};

using unpack_t = struct __unpack_t
{
    static constexpr char *name = "unpack";
};

template <typename T>
bool Process(const char *src, const char *dst)
{

    auto srcAbsPath = System::IO::Path::GetFullPath(gcnew System::String(src));
    auto detAbsPath = System::IO::Path::GetFullPath(gcnew System::String(dst));

    if constexpr (std::is_same_v<T, pack_t>)
    {
        if (!System::IO::Directory::Exists(srcAbsPath))
        {
            return false;
        }

        if (System::IO::Path::GetExtension(detAbsPath)->ToLower() != ".zip")
        {
            return false;
        }
    }
    else if constexpr (std::is_same_v<T, unpack_t>)
    {
        if (!System::IO::File::Exists(srcAbsPath))
        {
            return false;
        }

        if (System::IO::Path::GetExtension(srcAbsPath)->ToLower() != ".zip")
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if constexpr (std::is_same_v<T, pack_t>)
    {
        try
        {
            System::IO::Compression::ZipFile::CreateFromDirectory(srcAbsPath, detAbsPath);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    else if constexpr (std::is_same_v<T, unpack_t>)
    {
        try
        {
            System::IO::Compression::ZipFile::ExtractToDirectory(srcAbsPath, detAbsPath);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return false;
}

int PackArchive(const char *directory_name, const char *zipball_name)
{
    return static_cast<int>(Process<pack_t>(directory_name, zipball_name));
}

int UnPackArchive(const char *zipball_name, const char *directory_name)
{
    return static_cast<int>(Process<unpack_t>(zipball_name, directory_name));
}