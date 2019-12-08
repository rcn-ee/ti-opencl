#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */

#if _MSC_VER
#include "getopt.h"
#define strtok_r strtok_s
#else
#include <getopt.h>
#endif
#include <string.h>

#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

#include "file_manip.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ostream_iterator;

int opt_help      = 0;
int opt_verbose   = 0;
int opt_keep      = 0;
int opt_debug     = 0;
int opt_symbols   = 0;
int opt_lib       = 0;
int opt_link      = 0;
int opt_expsyms   = 0;
int opt_ar_lib    = 0;
int opt_link_opts = 0;
int opt_txt       = 0;
int opt_w         = 0;
int opt_Werror    = 0;
int opt_builtin   = 0;
int opt_tmpdir    = 0;
int opt_version   = 0;
int opt_alias     = 0;

string cl_options;
string cl_incdef;
string opts_other;
vector<string> files_clc;
vector<string> files_c;
string         files_a;
string         files_out;
string         files_other;
string         file_expsyms;

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)

void print_version()
{
    printf("OpenCL 1.2 TI product version "
           STRINGIZE2(_PRODUCT_VERSION)
           " (" STRINGIZE2(_BUILD_ID) ")\n\n");
}

/******************************************************************************
* void print_options()
******************************************************************************/
void print_options()
{
    cout << endl;

    if (opt_keep)      printf ("Option keep       : on\n");
    if (opt_debug)     printf ("Option debug      : on\n");
    if (opt_lib)       printf ("Option lib        : on\n");
    if (opt_link)      printf ("Option link       : on\n");
    if (opt_expsyms)   printf ("Option Export Syms: on\n");
    if (opt_ar_lib)    printf ("Option create lib : on\n");
    if (opt_link_opts) printf ("Option link opts  : on\n");
    if (opt_txt)       printf ("Option txt        : on\n");
    if (opt_w)         printf ("Option w          : on\n");
    if (opt_Werror)    printf ("Option Werror     : on\n");
    if (opt_alias)     printf ("Option alias      : on\n");
    if (opt_symbols)   printf ("Option symbols    : on\n");
    //if (opt_builtin) printf ("Option builtin: on\n");
    //if (opt_tmpdir)  printf ("Option tmpdir : on\n");

    cout << endl;

    cout << "CL C Options  : " << cl_options   << endl;
    cout << "Incls/Defines : " << cl_incdef    << endl;
    if (!files_clc.empty())
        cout << "CL C file     : " << files_clc[0] << endl;
    cout << "Output File   : " << files_out    << endl;
    cout << "Link Files    : " << files_other  << endl;
    cout << "Export Symbols File : " << file_expsyms  << endl;

    cout << endl;

    cout << "Ignored Opts  : " << opts_other   << endl;
    cout << "Ignored Files : ";
    if (files_clc.size() > 1)
        copy(files_clc.begin()+1, files_clc.end(), ostream_iterator<string>(cout, " "));

    copy(files_c.begin(), files_c.end(), ostream_iterator<string>(cout, " "));
    cout << endl << endl;
}

/******************************************************************************
* void print_help()
******************************************************************************/
void print_help()
{
    cout << endl;
    cout << "Usage: clocl [options] <OpenCL C file> [<link files>]" << endl;
    cout << endl;

    cout << "Options passed to clocl are either options to control" << endl;
    cout << "clocl behavior or they are documented OpenCL 1.2 build" << endl;
    cout << "options." << endl;
    cout << endl;
    cout << "The clocl behavior options are: " << endl;
    cout << "   -h, --help    : Print this help screen" << endl;
    cout << "   -v, --verbose : Print verbose messages" << endl;
    cout << "   -k, --keep    : Do not delete temp compilation files" << endl;
    cout << "   -g, --debug   : Generate debug symbols" << endl;
    cout << "   -t, --txt     : Generate object in header form" << endl;
    cout << "   -l, --lib     : Do not link. Stop after compilation." << endl;
    cout << "   -s, --symbols : Keep Symbols." << endl;
    cout << "   -a, --alias   : Assume kernel buffers alias each other" << endl;
    cout << "   --version     : Print OpenCL product." << endl;
    cout << endl;
    cout << "The OpenCL 1.2 build options. Refer to 1.2 spec for desc:" << endl;
    cout << "   -D<name>" << endl;
    cout << "   -D<name>=<val>" << endl;
    cout << "   -I<dir>" << endl;
    cout << "   -w" << endl;
    cout << "   -Werror" << endl;
    cout << "   -cl-single-precision-constant" << endl;
    cout << "   -cl-denorms-are-zero" << endl;
    cout << "   -cl-opt-disable" << endl;
    cout << "   -cl-mad-enable" << endl;
    cout << "   -cl-no-signed-zeros" << endl;
    cout << "   -cl-unsafe-math-optimizations" << endl;
    cout << "   -cl-finite-math-only" << endl;
    cout << "   -cl-fast-relaxed-math" << endl;
    cout << "   -cl-std=<val>" << endl;
    cout << "   -cl-kernel-arg-info" << endl;
    cout << endl;
    exit(-1);
}

/******************************************************************************
* void process_options(int argc, char **argv)
******************************************************************************/
void process_options(int argc, char **argv)
{
    int c;

    while (1)
    {
        static struct option long_options[] = {

            /*-----------------------------------------------------------------
            * clocl options
            *----------------------------------------------------------------*/
            {"help",        no_argument,        &opt_help,    'h' },
            {"verbose",     no_argument,        &opt_verbose, 'v' },
            {"keep",        no_argument,        &opt_keep,    'k' },
            {"debug",       no_argument,        &opt_debug,   'g' },
            {"symbols",     no_argument,        &opt_symbols, 's' },
            {"lib",         no_argument,        &opt_lib,     'l' },
            {"link",        no_argument,        &opt_link,    'z' },
            {"txt",         no_argument,        &opt_txt,     't' },
            {"builtin",     no_argument,        &opt_builtin, 'b' },
            {"tmpdir",      no_argument,        &opt_tmpdir,  'd' },
            {"alias",       no_argument,        &opt_alias,   'a' },
            {"version",     no_argument,        &opt_version,  1  },
            {"export-syms", required_argument,  &opt_expsyms,  0  },

            /*-----------------------------------------------------------------
            * opencl 1.2 options
            *----------------------------------------------------------------*/
            {"Werror",                       no_argument, 0,  0  },
            {"cl-std",                       required_argument, 0,  0  },
            {"cl-single-precision-constant", no_argument, 0, 0 },
            {"cl-denorms-are-zero",          no_argument, 0, 0 },
            {"cl-opt-disable",               no_argument, 0, 0 },
            {"cl-mad-enable",                no_argument, 0, 0 },
            {"cl-no-signed-zeros",           no_argument, 0, 0 },
            {"cl-unsafe-math-optimizations", no_argument, 0, 0 },
            {"cl-finite-math-only",          no_argument, 0, 0 },
            {"cl-fast-relaxed-math",         no_argument, 0, 0 },
            {"cl-kernel-arg-info",           no_argument, 0, 0 },
            {"create-library",               no_argument, 0, 0 },
            {"enable-link-options",          no_argument, 0, 0 },
            {0,                              0,           0, 0 }
        };

        int this_option_optind = optind ? optind : 1;
        int option_index = 0;

        opterr = 0; // prevent getopt from printing warnings

       c = getopt_long_only(argc, argv, "-tadglzvwsI:D:", long_options,
                            &option_index);
       if (c == -1) break;

       switch (c)
       {
           case 0:
               {
                string name(long_options[option_index].name);

                if (name == "help"    || name == "verbose"   ||
                    name == "keep"    || name == "debug"     ||
                    name == "lib"     || name == "txt"       ||
                    name == "link"    ||
                    name == "builtin" || name == "tmpdir"    ||
                    name == "alias"   || name == "symbols"
                   ) break;

                if (name == "cl-std")
                {
                    cl_options += " -cl-std=";
                    cl_options += optarg;
                    break;
                }

                if (name == "export-syms")
                {
                    file_expsyms += optarg;
                    opt_expsyms  = 1;
                    break;
                }

                if (name == "Werror")         opt_Werror = 1; // fall-through
                if (name == "cl-opt-disable") opt_debug  = 1; // fall-through

                if (name == "create-library")      opt_ar_lib = 1;
                if (name == "enable-link-options") opt_link_opts = 1;

                cl_options += " ";
                cl_options += argv[this_option_optind];
                break;
               }

           case 1:
               {
                 string fname(argv[this_option_optind]);
                 string ext(fs_ext(fname));

                 if      (ext == ".clc") files_clc.push_back(fname);
                 else if (ext == ".cl")  files_clc.push_back(fname);
                 else if (ext == ".a")   { files_a += fname; files_a += " ";}
                 else if (ext == ".c")   files_c.push_back(fname);
                 else if (ext == ".out") files_out = fname;
                 else                    { files_other += fname; files_other += " "; }

                 break;
               }

           case 't': opt_txt     = 1; break;
           case 'd': opt_tmpdir  = 1; break;
           case 'l': opt_lib     = 1; break;
           case 'z': opt_link    = 1; break;
           case 'v': opt_verbose = 1; break;
           case 'a': opt_alias   = 1; break;
           case 'g': opt_debug   = 1; break;
           case 's': opt_symbols = 1; break;
           case 'w': opt_w = 1; cl_options += " -w"; break;

           case 'D':
           case 'I':
                     cl_incdef += " -";
                     cl_incdef += c;
                     cl_incdef += optarg;
                     break;

           case '?':
                opts_other += " ";
                opts_other += argv[this_option_optind];
                break;

           default:
                opts_other += " -";
                opts_other += c;
                break;
        }
    }

    /*-------------------------------------------------------------------------
    * __TI_OCL: if defined, then it is TI OpenCL implementation
    * __TI_OCL_VERSION: 0xXXYYZZWWU, e.g. version 0.12.0 => 0x00120000U
    *------------------------------------------------------------------------*/
    cl_incdef += " -D__TI_OCL";
    char tiocl_ver[64];
    sprintf(tiocl_ver, STRINGIZE2(_PRODUCT_VERSION));
    int ver[4] = { 0, 0, 0, 0 };
    char *firstcall = tiocl_ver, *token, *saveptr;
    for (int i = 0; i < 4; ++i, firstcall = NULL)
    {
        token = strtok_r(firstcall, ".", &saveptr);
        if (token == NULL)  break;
        ver[i] = atoi(token);
        if (ver[i] > 99) ver[i] = 99;  // report error?
    }
    snprintf(tiocl_ver, sizeof(tiocl_ver),
            " -D__TI_OCL_VERSION=0x%02d%02d%02d%02dU",
            ver[0], ver[1], ver[2], ver[3]);
    cl_incdef += tiocl_ver;
    // Otherwise, clang at -O0 will throw those inline function away
    if (opt_debug)  cl_incdef += " -Dinline= ";

    if (opt_version) print_version();
    if (opt_verbose) print_options();
    if (opt_help)    print_help();

    cl_options += " ";
    cl_options += cl_incdef;
}
