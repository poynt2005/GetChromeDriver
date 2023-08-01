#ifndef __ZIPPER_UTIL_H__
#define __ZIPPER_UTIL_H__

#include <string>
#include <memory>

#ifdef EXPORTZIPPERAPI
#define ZIPPERAPI __declspec(dllexport)
#else
#define ZIPPERAPI __declspec(dllimport)
#endif

class ZIPPERAPI Zipper
{
public:
    Zipper();
    ~Zipper();

    bool PackArchive(const std::string &directoryName, const std::string zipballName) const;
    bool UnPackArchive(const std::string zipballName, const std::string &directoryName) const;

    std::string GetErrorCode() const { return errorCode; }

private:
    mutable std::string errorCode;

    bool isServenZipReady = false;

    bool ExtractSevenZipRuntime();
    void ClearSevenZipRuntime() const;

    template <typename T>
    bool CreateServenZipJob(const std::string &srcPath, const std::string &dstPath) const;

    std::string exePath;
};

#endif