#!/usr/bin/python
# #!/usr/local/bin/python2.6-2.6.4
# -*- coding: utf-8 -*-
#
# Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holder nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# embed.py <outfile> <filenames..>
# <filenames> => <outfile>

import sys

outfile = open(sys.argv[1], 'w')
name = sys.argv[1].split('/')[-1].replace('.embed.h', '').replace('.', '_')

data = ''

for i in xrange(len(sys.argv) - 1):
    infile = open(sys.argv[i + 1], 'rb')
    data += infile.read()

# Header
outfile.write('#ifndef __%s__\n' % name.upper())
outfile.write('#define __%s__\n' % name.upper())
outfile.write('\n')
outfile.write('const char embed_%s[] =\n' % name)

# Write it in chunks of 80 chars :
# |    "\x00..."            (4+1+1 + 4*chars ==> chars = 18)
index = 0

for c in data:
    if index == 0:
        outfile.write('    "')

    outfile.write('\\x%s' % ('%x' % ord(c)).rjust(2, '0'))
    index += 1

    if index == 18:
        index = 0
        outfile.write('"\n')

# We may need to terminate a line
if index != 0:
    outfile.write('";\n')
else:
    outfile.write(';\n')     # Alone on its line, poor semicolon

# Footer
outfile.write('\n')
outfile.write('#endif\n')

infile.close()
outfile.close()
