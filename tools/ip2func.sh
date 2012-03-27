#!/bin/bash
#===============================================================================
#
#          FILE:  ip2func.sh
# 
#         USAGE:  ./ip2func.sh 
# 
#   DESCRIPTION:  
# 
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR:   (), 
#       COMPANY:  
#       VERSION:  1.0
#       CREATED:  27.03.2012 13:44:41 CEST
#      REVISION:  ---
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

