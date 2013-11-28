#!/bin/bash
#===============================================================================
#
#          FILE:  ip2func.sh
# 
#         USAGE:  ./ip2func.sh 
# 
#   DESCRIPTION:  convert IP (instr. pointer) address to function name and assembler output. 
#                 Currently only 32 bit 
# 
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR:  Georg Wassen  (gw), wassen@lfbs.rwth-aachen.de
#       COMPANY:  RWTH Aachen University
#       VERSION:  1.0
#       CREATED:  27.03.2012 13:44:41 CEST
#      REVISION:  ---
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
#===============================================================================

function usage()
{
    echo "$(basename $0) {32|64} ip"
}

if [ $# -lt 2 ]; then
    usage
    exit
fi

BITS=$1

if [ $BITS -ne 32 -a $BITS -ne 64 ]; then
    usage
    exit
fi

if [ $BITS -ne 32 ]; then
    echo "Sorry, only 32 bit support, so far."
    exit
fi

IP=$2

#echo BITS = $BITS
#echo IP   = $IP = $(($IP))


while read NUM FUNC; do 
    if [ $(($IP)) -lt $(($NUM)) ]; then
        echo $PREV_NUM - $PREV_FUNC
        break
    fi
    PREV_NUM=$NUM
    PREV_FUNC=$FUNC
    #echo $NUM: $FUNC 
done < <(sed -n 's/^\s*\(0x[0-9a-f]*\)\s*\([a-zA-Z_][a-zA-Z0-9_]*\)$/\1 \2/p' kernel32.map)

D=$(nm -o *.o32 | grep $PREV_FUNC | sed 's/:/ /')
FILE=$(echo $D|cut -d' ' -f1)
OFFSET=0x$(echo $D|cut -d' ' -f2)

echo $FILE $OFFSET

printf "0x%x after begin of $PREV_FUNC\n" $(( $IP - $PREV_NUM  ))
FILE_OFFSET=$(( $OFFSET + $IP - $PREV_NUM ))
printf "0x%x in $FILE \n" $FILE_OFFSET

objdump -M intel -d $FILE | grep -A 10 -B 10 $(printf '%x' $FILE_OFFSET)

