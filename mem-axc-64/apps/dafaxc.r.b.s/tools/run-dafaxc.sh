#!/bin/bash
# dir=164.gzip.tr
# for dir in `ls  -d *.tr `
# do
#   mkdir -p ../dafaxc_runs
#   pushd $dir 
#   echo $dir
#
#   for dotfile in `ls *.dot`
#   do
#     echo $dotfile
#     mkdir -p ../../dafaxc_runs/$dir
#     filename="${dotfile%.*}"
#     # /media/vnaveen0/shared/vm-shared/work/new-daxac/dafaxc/build/bin/dafaxc -i 
#     /media/vnaveen0/Windows/Users/naveen/wind_drive/sfu/nachos/Nachos/build_repo/dafaxc/bin/dafaxc -i $dotfile > ../../dafaxc_runs/$dir/${filename}.stdout
#
#   done
#   popd
# done

/media/vnaveen0/Windows/Users/naveen/wind_drive/sfu/nachos/Nachos/build_repo/dafaxc/bin/dafaxc -i $dotfile > stdout

