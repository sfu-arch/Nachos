#!/bin/bash

for dir in `ls  -d *.tr `
do
  mkdir -p ../svg
  pushd $dir 
  # cd $dir
  echo $dir

  for dotfile in `ls *.dot`
  do
    echo 'pwd'
    mkdir -p ../../svg/$dir
    filename="${dotfile%.*}"
    dot -Tsvg $dotfile > ../../svg/$dir/${filename}.svg
  done
  popd
  # cd ..
done

# for dir in `ls  *.tar.gz`
# do
#   tar -xvf $dir
# done
