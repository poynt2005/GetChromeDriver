#ifndef __ZIPPER_UTIL_H__
#define __ZIPPER_UTIL_H__

#ifdef EXPORTZIPPERAPI
#define ZIPPERAPI __declspec(dllexport)
#else
#define ZIPPERAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    ZIPPERAPI int PackArchive(const char *directory_name, const char *zipball_name);
    ZIPPERAPI int UnPackArchive(const char *zipball_name, const char *directory_name);
#ifdef __cplusplus
}
#endif

#endif