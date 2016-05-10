#include "file_manip.h"
#if defined(_MSC_VER)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

size_t find_last_path_separator(const std::string& path)
{
    size_t pos = path.rfind('/');

    #if defined(_MSC_VER)
    if (pos == std::string::npos)
    {
        pos = path.rfind('\\');
    }
    #endif

    return pos;
}

bool fs_exists(std::string path)
{
    #if defined(_MSC_VER)
    /*------------------------------------------------------------------------*/
    /* On Windows, the stat() function will return false for paths with       */
    /* with trailing slashes. This is according to the MSDN explanation       */
    /* _stat(). Which can be found here:                                      */
    /*    http://msdn.microsoft.com/en-us/library/14h5k7ff.aspx               */
    /* We do accept paths with trailing slashes as option                     */
    /* arguments, so this function should return true for these paths.        */
    /*------------------------------------------------------------------------*/
    while (path[path.length() - 1] == '\\' ||
           path[path.length() - 1] == '/')
        path.erase(path.length() - 1);
    #endif

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == 0) return true;
    else                                   return false;
}

std::string fs_filename(std::string path)
{
    size_t name_begin = find_last_path_separator(path);
    if (name_begin == std::string::npos) return path;
    return path.substr(name_begin+1, path.size()-name_begin+1);
}

std::string fs_stem(std::string path)
{
    path = fs_filename(path);
    size_t ext_begin = path.rfind(".");
    if (ext_begin == std::string::npos) return path;
    return path.substr(0, ext_begin);
}

std::string fs_ext(std::string path)
{
    size_t ext_begin = path.rfind(".");
    if (ext_begin == std::string::npos) return "";
    return path.substr(ext_begin, path.size()-ext_begin);
}

std::string fs_path(std::string path)
{
    size_t path_end = find_last_path_separator(path);
    if (path_end == std::string::npos) return "";
    return path.substr(0, path_end+1);
}

std::string fs_replace_extension(std::string path, std::string ext)
{
    if (fs_ext(path) == "") return path;

    path = fs_path(path) + fs_stem(path);

    if (ext[0] == '.') return path + ext;
    else               return path + "." + ext;
}

// Returned string includes trailing path separator
// "E.g. c:\cygwin64\tmp\"
std::string fs_get_tmp_folder()
{
#if defined(_MSC_VER)
    char tmp_path[MAX_PATH];
    int count = GetTempPath(MAX_PATH, tmp_path);

    return std::string(tmp_path);
#else
    return std::string("/tmp/");
#endif
}

#ifdef TEST
#include <iostream>
int main()
{
    std::cout << "fs_get_tmp_folder: " << fs_get_tmp_folder() << std::endl;
    std::cout << "fs_path: " << fs_path(fs_get_tmp_folder()) << std::endl;
    std::cout << "fs_ext:" << fs_ext("C:\\cygwin\\tmp\\a.asm") << std::endl;
    std::cout << "fs_stem:" << fs_stem("C:\\cygwin\\tmp\\a.asm") << std::endl;
    std::cout << "fs_filename:" << fs_filename("C:\\cygwin\\tmp\\a.asm") << std::endl;
}
#endif