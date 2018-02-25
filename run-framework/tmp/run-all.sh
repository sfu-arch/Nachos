#!/bin/bash

# Each benchmark has a folder, each folder contains
# bench.sh which contains benchmark specific variables.
# run.sh is copied into each folder and then executed,
# and is benchmark agnostic. 

source apps.sh

BEGIN_TIME=`date`
for app in ${APPS[@]}; do
   cp run.sh ${app}/ 
   #cp pre-run.sh ${app}/ 
   # cp pre-run.sh ${app}/ 
   pushd ${app} > /dev/null
   # bash pre-run.sh &
   bash run.sh &
   popd > /dev/null
done
wait

END_TIME=`date`
echo -e "Beg: ${BEGIN_TIME}\nEnd: ${END_TIME}" | mail -s "N5 runs complete" $(whoami)@cs.sfu.ca 
