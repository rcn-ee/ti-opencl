#include "file_manip.h"
#ifndef _MSC_VER 
#include <unistd.h>
#else
#include <io.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define access  _access
#define stat    _stat
#define F_OK    0
#define R_OK    04
#define S_ISDIR(A) ((A) && _S_IFDIR)
#endif

bool fs_exists(std::string path) 
{ 
    return (access(path.c_str(), F_OK) == 0); 
}

std::string fs_filename(std::string path) 
{
    unsigned int name_begin = path.rfind("/");
    if (name_begin == std::string::npos) return path;
    return path.substr(name_begin+1, path.size()-name_begin+1);
}

std::string fs_stem(std::string path)
{
    path = fs_filename(path);
    unsigned int ext_begin = path.rfind(".");
    if (ext_begin == std::string::npos) return path;
    return path.substr(0, ext_begin);
}

std::string fs_ext(std::string path) 
{  
    unsigned int ext_begin = path.rfind(".");
    if (ext_begin == std::string::npos) return "";
    return path.substr(ext_begin, path.size()-ext_begin);
}

std::string fs_path(std::string path)
{  
    unsigned int path_end = path.rfind("/");
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
