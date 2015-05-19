#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <string>

extern int opt_help;
extern int opt_verbose;
extern int opt_version;
extern int opt_keep;
extern int opt_debug;
extern int opt_symbols;
extern int opt_lib;
extern int opt_txt;
extern int opt_w;
extern int opt_Werror;
extern int opt_builtin;
extern int opt_tmpdir;
extern int opt_alias;

extern std::string cl_options;
extern std::string cl_incdef;
extern std::vector<std::string> files_clc;
extern std::vector<std::string> files_c;
extern std::string              files_other;

void process_options(int argc, char **argv);

#endif //_OPTIONS_H_
