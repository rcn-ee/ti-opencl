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
extern int opt_link;
extern int opt_expsyms;
extern int opt_ar_lib;
extern int opt_link_opts;
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
extern std::string              files_a;
extern std::string              files_out;
extern std::string              files_other;
extern std::string              file_expsyms;

void process_options(int argc, char **argv);

#endif //_OPTIONS_H_
