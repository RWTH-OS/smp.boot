# vim: list ts=8

# Parameters:
#   1. base address
#   2. input file (64 bit kernel)
#   3. output file (section)

# this script was extracted from inline python (here-document) in build64.sh (r552)
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
regex=r"\[\s*\d+\]\s*(?!NULL)(\S+)\s+(PROGBITS|NOBITS)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)"
baseaddress=int(sys.argv[1], 0)
#print baseaddress
k64f = open(sys.argv[2], "r")
f = open(sys.argv[3], "w")
for line in sys.stdin:
	r = re.search(regex, line)
	if not r:
		continue
	section, stype, LMA, offset, size = r.groups()
	LMA, offset, size = map(lambda s: int(s, 16), (LMA, offset, size))
	if LMA < baseaddress:
		raise ValueError("section ('%s' offset=0x%08x size=0x%08x ) at address < 0x%x" % (section, offset, size, baseaddress))
	k64f.seek(offset)
	if stype == "PROGBITS":
		f.seek(LMA-baseaddress)
		f.write(k64f.read(size))
f.close()

