# vim: list ts=8

# Parameters:
#   1. symbol list
#   3. output file (symbols)

# this script is extracted from inline python (here-document) in build64.sh (r552)
# which in turn was derived from http://wiki.osdev.org/X86-64
# Original author: osdev.org user Lithium26, 2010-01-22
#
# Copyright (c) 2011, Georg Wassen, RWTH Aachen University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#    * Neither the name of the University nor the names of its contributors
#      may be used to endorse or promote products derived from this
#      software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import re, sys
regex=r"\s*\:\s+([0-9a-fA-F]+)\s+\d+\s+\S+\s+\S+\s+\S+\s+\S+\s+(\S+)"
syms=filter(len,sys.argv[1].split(" "))
f = open(sys.argv[2], "w")
for line in sys.stdin:
	r = re.search(regex, line)
	if not r:
		continue
	val, sym = r.groups()
	if not sym in syms:
		continue
	val = int(val, 16)
	if val > 0xffffffff:
		raise ValueError("symbol value must be below 0xffffffff limit")
	f.write('"%s" = %s;\n' % (sym, hex(val)))
f.close()
