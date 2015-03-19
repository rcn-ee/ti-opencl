#!/usr/bin/python
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


# builtins.py <def> <outdir>

import sys

class Function:
    class Arg:
        def __init__(self, name, t):
            self.name = name
            self.t = t

    KIND_BUILTINS_IMPL = 0      # static function in builtins.cpp
    KIND_BUILTINS_DEF = 1       # if (name == '__cpu_$name') return (void *)&name;
    KIND_STDLIB_IMPL = 2        # OpenCL C function in stdlib.c
    KIND_STDLIB_DEF = 3         # Header in stdlib.h
    KIND_STDLIB_STUB = 4        # OpenCL C stub in stdlib.c: calls __cpu_$name
    KIND_STDLIB_STUB_DEF = 5    # __cpu_$name declared in stdlib.c

    def __init__(self, name, native):
        self.name = name
        self.native = native

        self.args = []   # Array <Arg>
        self.types = []  # Array <str>
        self.return_type = ''
        self.body = ''

    def set_return_type(self, ty):
        self.return_type = ty

    def append_body(self, body):
        self.body += body

    def add_arg(self, name, ty):
        self.args.append(self.Arg(name, ty))

    def add_type(self, ty):
        self.types.append(ty)

    def mangled_name(self, current_type):
        return_type = self.process_type_name(current_type, self.return_type)

        rs = return_type + '_' + self.name
        first = True

        for a in self.args:
            if first:
                rs += '_'
            first = False

            arg_type = self.process_type_name(current_type, a.t)
            rs += arg_type.replace('*', 'p')

        return rs

    def process_type_name(self, current_type, type_name):
        # Current vector dimension
        vecdim = '1'

        if current_type[-1].isdigit():
            if current_type[-2].isdigit():
                vecdim = current_type[-2:]
            else:
                vecdim = current_type[-1]

        # $vecdim expansion
        return type_name.replace('$vecdim', vecdim).replace('$type', current_type)

    def arg_list(self, current_type, handle_first_arg):
        rs = ''
        first = True
        append_arg = None

        # We may need a first "result" arg
        if handle_first_arg:
            return_type = self.process_type_name(current_type, self.return_type)

            if return_type[-1].isdigit():
                # Return is a vector
                append_arg = self.Arg('result', return_type)

        if append_arg:
            args = [append_arg] + self.args
        else:
            args = self.args

        for arg in args:
            # Resolve type
            arg_type = self.process_type_name(current_type, arg.t)

            if arg_type[0] == '*':
                arg_ptr = True
                arg_type = arg_type[1:]
            else:
                arg_ptr = False

            # We need to pass vector arguments as pointers
            arg_vector = False
            if handle_first_arg:
                arg_vector = arg_type[-1].isdigit()
                arg_type = arg_type.rstrip('0123456789')

            # Build the string
            if not first:
                rs += ', '
            first = False

            rs += arg_type + ' '

            if arg_vector or arg_ptr:
                rs += '*'

            rs += arg.name

        return rs

    def write(self, current_type, kind):
        # Template:
        # (static) $ret_type $name($args) {
        # $body
        # }
        rs = ''

        if kind == self.KIND_BUILTINS_IMPL:
            rs = 'static '
        elif kind == self.KIND_BUILTINS_DEF:
            rs += '    else if (name == "__cpu_' + self.mangled_name(current_type) + '")\n'
            rs += '        return (void *)&' + self.mangled_name(current_type) + ';\n'
            return rs

        # Calculate return type
        return_type = self.process_type_name(current_type, self.return_type)

        if (kind == self.KIND_BUILTINS_IMPL or kind == self.KIND_STDLIB_STUB_DEF) \
            and return_type[-1].isdigit():
            return_type = 'void' # We'll use a 'result' argument

        rs += return_type + ' '

        # Append mangled name if needed
        if kind == self.KIND_BUILTINS_IMPL:
            rs += self.mangled_name(current_type)
        elif kind == self.KIND_STDLIB_STUB_DEF:
            rs += '__cpu_' + self.mangled_name(current_type)
        else:
            # No need to mangle the name, but add OVERLOAD
            rs += 'OVERLOAD ' + self.name

        # Print function args
        rs += '('
        rs += self.arg_list(current_type, kind == self.KIND_BUILTINS_IMPL or \
                                          kind == self.KIND_STDLIB_STUB_DEF)
        rs += ')'

        # If only a declaration, end it
        if kind == self.KIND_STDLIB_DEF or kind == self.KIND_STDLIB_STUB_DEF:
            rs += ';\n'
            return rs

        # Add the body
        rs += '\n{\n'

        if kind == self.KIND_STDLIB_STUB:
            # Special body : call __cpu_$name
            return_is_vector = return_type[-1].isdigit()
            if return_is_vector:
                # Need to create a temporary
                rs += '    ' + return_type + ' result;\n'
                rs += '\n'

            # Call the cpu stub
            rs += '    '
            if not return_is_vector:
                rs += 'return '

            rs += '__cpu_' + self.mangled_name(current_type) + '('

            # Pass the result if needed
            first = True
            if return_is_vector:
                rs += '(' + return_type.rstrip('0123456789') + ' *)&result'
                first = False

            # Append the args
            for arg in self.args:
                # Resolve type
                arg_type = self.process_type_name(current_type, arg.t)

                arg_ptr = False
                if arg_type[0] == '*':
                    arg_type = arg_type[1:]
                    arg_ptr = True
                    
                arg_vector = arg_type[-1].isdigit()

                if not first:
                    rs += ', '
                first = False

                # We need to pass vector arguments as pointers
                if arg_vector:
                    rs += '(' + arg_type.rstrip('0123456789') + ' *)'
                    if not arg_ptr:
                        rs += '&'

                rs += arg.name

            # End the call
            rs += ');\n'

            if return_is_vector:
                rs += '\n    return result;\n'

            rs += '}\n\n'
        else:
            # Simply copy the body
            vecdim = '1'

            if current_type[-1].isdigit():
                if current_type[-2].isdigit():
                    vecdim = current_type[-2:]
                else:
                    vecdim = current_type[-1]

            rs += self.body.replace('$type', current_type) \
                           .replace('$vecdim', vecdim)
            rs += '\n}\n\n'

        return rs

class Generator:
    builtins_impl_file = 'builtins_impl.h'    # static functions
    builtins_def_file = 'builtins_def.h'      # if () in getBuiltin
    stdlib_impl_file = 'stdlib_impl.h'        # stdlib.c functions
    stdlib_def_file = 'stdlib_def.h'          # stdlib.h definitions

    def __init__(self, out_path):
        self.out_path = out_path

        # Buffers
        self.builtins_impl_buffer = ''
        self.builtins_def_buffer = ''
        self.stdlib_impl_buffer = ''
        self.stdlib_def_buffer = ''

    def add_function(self, function):
        for t in function.types:
            if function.native:
                self.stdlib_impl_buffer += function.write(t, function.KIND_STDLIB_STUB_DEF)
                self.stdlib_impl_buffer += function.write(t, function.KIND_STDLIB_STUB)
                self.stdlib_def_buffer += function.write(t, function.KIND_STDLIB_DEF)
                self.builtins_impl_buffer += function.write(t, function.KIND_BUILTINS_IMPL)
                self.builtins_def_buffer += function.write(t, function.KIND_BUILTINS_DEF)
            else:
                self.stdlib_def_buffer += function.write(t, function.KIND_STDLIB_DEF)
                self.stdlib_impl_buffer += function.write(t, function.KIND_STDLIB_IMPL)

    def write(self):
        of = open(self.out_path + '/' + self.stdlib_def_file, 'w')
        of.write(self.stdlib_def_buffer)
        of.close()

        of = open(self.out_path + '/' + self.stdlib_impl_file, 'w')
        of.write(self.stdlib_impl_buffer)
        of.close()

        of = open(self.out_path + '/' + self.builtins_def_file, 'w')
        of.write(self.builtins_def_buffer)
        of.close()

        of = open(self.out_path + '/' + self.builtins_impl_file, 'w')
        of.write(self.builtins_impl_buffer)
        of.close()

class Parser:
    def __init__(self, generator, def_file_name):
        self.generator = generator
        self.def_file_name = def_file_name

        self.defs = {}

    def replace_variable(self, token):
        result = []

        if token[0] == '$':
            for tok in self.defs[token[1:]]:
                result.extend(self.replace_variable(tok))
        else:
            result.append(token)

        return result

    def parse(self):
        def_file = open(self.def_file_name, 'rb')
        current_function = None

        for line in def_file:
            if current_function:
                # End if we encounter an end
                if line.startswith('end'):
                    self.generator.add_function(current_function)
                    current_function = None
                else:
                    # Add a line to the body
                    current_function.append_body(line)
            else:
                line = line.strip()
                tokens = line.split(' ')
                tok = tokens[0]

                if tok == 'def':
                    # A definition : def <variable> : [values]
                    name = tokens[1]
                    values = []

                    for token in tokens[3:]:
                        values.extend(self.replace_variable(token))

                    self.defs[name] = values
                elif tok == 'func' or tok == 'native':
                    # Function : func|native <ret_type> <name> [types] : [args]
                    current_function = Function(tokens[2], \
                                                tokens[0] == 'native')

                    current_function.set_return_type(tokens[1])

                    # Explore the types and args
                    in_types = True

                    for token in tokens[3:]:
                        if token == ':':
                            in_types = False
                        elif in_types:
                            for ty in self.replace_variable(token):
                                current_function.add_type(ty)
                        else:
                            # Parameters
                            parts = token.split(':')
                            current_function.add_arg(parts[0], parts[1])

        def_file.close()

if __name__ == '__main__':
    def_file = sys.argv[1]
    out_dir = sys.argv[2]

    gen = Generator(out_dir)
    parser = Parser(gen, def_file)

    parser.parse()
    gen.write()
