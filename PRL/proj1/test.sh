#!/bin/bash

if [ $# -lt 1 ];then 
    numbers=21;
else
    numbers=$1;
fi;

#preklad cpp zdrojaku
mpic++ --prefix /usr/local/share/OpenMPI -o parsplit parsplit.c


#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers

#spusteni
mpirun --prefix /usr/local/share/OpenMPI -np 7 parsplit

#uklid
rm -f parsplit numbers
