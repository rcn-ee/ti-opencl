/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "test_platform.h"
#include "test_device.h"
#include "test_context.h"
#include "test_commandqueue.h"
#include "test_mem.h"
#include "test_kernel.h"
#include "test_program.h"
#include "test_builtins.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int n_failed_tests;
    Suite *s = NULL;

    if (argc < 2) {
        printf("test <test> [nofork]\n");
        return EXIT_FAILURE;
    }
    
#define TESTSUITE(name, string)                         \
    if (!strcmp(string, argv[1])) {                     \
        s = suite_create(string);                       \
        suite_add_tcase(s, cl_##name##_tcase_create()); \
    }
    
    TESTSUITE(platform, "platform");
    TESTSUITE(device, "device");
    TESTSUITE(context, "context");
    TESTSUITE(commandqueue, "commandqueue");
    TESTSUITE(mem, "mem");
    TESTSUITE(kernel, "kernel");
    TESTSUITE(program, "program");
    TESTSUITE(builtins, "builtins");

    if (s == NULL) {
        printf("test case %s does not exist\n", argv[1]);
        return EXIT_FAILURE;
    }

    SRunner *sr = srunner_create(s);

    if (argc == 3 && !strcmp("nofork", argv[2]))
        srunner_set_fork_status (sr, CK_NOFORK);

    srunner_run_all(sr, CK_NORMAL);

    n_failed_tests = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (n_failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
