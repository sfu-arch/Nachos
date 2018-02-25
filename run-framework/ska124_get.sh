source apps.sh

ROOT=`pwd`

for x in ${APPS[@]}; do
  pushd ${x} > /dev/null
  source bench.sh          
  for f in ${FUNCTIONS[@]}; do
    echo ${x} ${f}
    #pushd ${f} > /dev/null
    #cov=`python ${ROOT}/ska124_path.py epp-sequences.txt | sed -n '5p'`
    #echo ${x} ${cov} 
    #popd > /dev/null
  done
  popd > /dev/null
done

