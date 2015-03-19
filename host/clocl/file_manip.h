#ifndef _FILE_MANIP_H_
#define _FILE_MANIP_H_
#include <string>

bool        fs_exists           (std::string path);
std::string fs_filename         (std::string path);
std::string fs_stem             (std::string path);
std::string fs_ext              (std::string path);
std::string fs_path             (std::string path);
std::string fs_replace_extension(std::string path, std::string ext);

#endif // _FILE_MANIP_H_
