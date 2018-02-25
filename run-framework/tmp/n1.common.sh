grep_edges() {
    #---- MEAD STATS ---------------------------------------------------------
    if [ -f "mead.graph.${n}.stats.txt" ]; then
      #per iteration stats
      s1=`egrep "num-orig-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`
      s2=`egrep "num-added-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`
      s3=`egrep "num-dataflow-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`

      echo $s1 $s2 $s3
    fi
}




#----------------------------------------------------------------------------------------
may_prof_stats () {

  no_count=`cut -d' ' -f 4  $1 | grep -v "^0$" | wc -l`
  yes_count=`cut -d' ' -f 5  $1 | grep -v "^0$" | wc -l`

# echo "toal_no_conflict_edges_are_zero: ${MUSTZ_count} total_must_conflicts_edges_are_zero: ${NOZ_count} total_SPLIT_cases: ${NZ_count} "

b=`egrep "num-no-alias" aew.$2.stats.txt | cut -d' ' -f2`
c=`egrep "num-must-alias" aew.$2.stats.txt | cut -d' ' -f2`
h=`egrep "num-ipaa-no-alias" aew.$2.stats.txt | cut -d' ' -f2`
i=`egrep "num-partial-alias" aew.$2.stats.txt | cut -d' ' -f2`


no_conflict=`echo "$b + $h + $no_count" |bc -l`
yes_conflict=`echo "$c +$i + $yes_count" |bc -l`
# echo "$3 $2 $no_conflict $yes_conflict $NZ_count"
echo "$3 $no_conflict $yes_conflict"
}

#----------------------------------------------------------------------------------------
run_longest_path() {
  #./longest_path.py dfg.__offload_func_862567.dot latency.csv 179.art

  bench=$1
  # bench=179.art
  dotbase=$DOTBASE/${bench}
  pushd $dotbase > /dev/null
  dotfile=`ls ldst.dfg.*`
  dotfile2=`ls mead.dfg.*`
  # echo $dotfile
  # echo $dotfile2
  popd >/dev/null

  tool=dfg

  echo "$1 dfg"

  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    bindir=/home/nvedula/summer2016/mem-axc-64/tools/python/longest_path
    $bindir/longest_path.py $dotbase/$dotfile $bindir/latency.csv $bench >${tool}.longest_path_${ID}.stats.txt __offload_func_${ID}.aa.txt 
  done

  tool=mead

  echo "$1 mead"

  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    bindir=/home/nvedula/summer2016/mem-axc-64/tools/python/longest_path
    $bindir/longest_path.py $dotbase/$dotfile2 $bindir/latency.csv $bench >${tool}.longest_path_${ID}.stats.txt __offload_func_${ID}.aa.txt 
  done
}
#---------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------
grep_all_AAstats() {
  #$1 = x : application name

  #Cumm. Global Vars
  #MEAD
  total_orig_edges=0
  total_added_edges=0

  #NAIVE AA
  total_naive_may=0
  total_naive_no=0
  total_naive_must=0
  total_naive_total=0

  #IPA AA
  total_ipaa_no=0
  total_ipaa_may=0

  for n in {0..4}; do 

    #---- MEAD STATS ---------------------------------------------------------
    if [ -f "mead.graph.${n}.stats.txt" ]; then
      #per iteration stats
      s1=`egrep "num-orig-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`
      s2=`egrep "num-added-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`
      s3=`egrep "num-dataflow-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`

      #cummulative stats for all top 5 paths
      total_orig_edges=$((total_orig_edges + s1))
      total_added_edges=$((total_added_edges + s2))
    fi

    #---- NAIVE AA STATS ---------------------------------------------------------
    if [ ! -f "aew.${n}.stats.txt" ]; then
      continue
    fi


    # Per iteration stats
    a=`egrep "num-aa-pairs" aew.${n}.stats.txt | cut -d' ' -f2`
    g=`egrep "num-ld-ld-pairs" aew.${n}.stats.txt | cut -d' ' -f2`
    b=`egrep "num-no-alias" aew.${n}.stats.txt | cut -d' ' -f2`
    c=`egrep "num-must-alias" aew.${n}.stats.txt | cut -d' ' -f2`
    d=`egrep "num-partial-alias" aew.${n}.stats.txt | cut -d' ' -f2`
    e=`egrep "num-may-alias-naive" aew.${n}.stats.txt | cut -d' ' -f2`
    h=`egrep "num-ipaa-no-alias" aew.${n}.stats.txt | cut -d' ' -f2`
    k=`egrep "num-ipaa-may-alias" aew.${n}.stats.txt | cut -d' ' -f2`
    # echo $a $b $c $d $e $g $h $k $s1 $s2
    naive_may=${e}
    naive_no=${b}
    naive_must=`echo "$c+$d" | bc -l`
    naive_total=`echo "$naive_no + $naive_may + $naive_must" | bc -l`
    naive_may_from_total=0

    # Cummulative stats for all top 5 paths
    total_naive_may=$((total_naive_may + naive_may))
    total_naive_no=$((total_naive_no + naive_no))
    total_naive_must=$((total_naive_must + naive_must))
    total_naive_total=$((total_naive_total + naive_total))




    #---- IPAA STATS ---------------------------------------------------------
    ipaa_no=${h}
    ipaa_may=${k}

    # Cummulative stats for all top 5 paths
    total_ipaa_no=$((total_ipaa_no + ipaa_no))
    total_ipaa_may=$((total_ipaa_may + ipaa_may))

  done


  #Cummulative stats  for top 5 paths

  #---- MEAD STATS ---------------------------------------------------------

  # echo $1 $total_orig_edges $total_added_edges 
  echo "${1} ${total_orig_edges} ${total_added_edges}" >> ${ROOT}/simplified.csv 


  #---- NAIVE STATS ---------------------------------------------------------
  total_naive_may_from_total=0
  if [  ${total_naive_total} -ne 0  ]; then

    total_naive_fraction=`echo "$total_naive_may/$total_naive_total*100" | bc -l`
    total_naive_may_from_total=${total_naive_fraction}
  fi

  echo "${1} ${total_naive_may} ${total_naive_no} ${total_naive_must}" >> ${ROOT}/naiveAA.csv 


  #---- IPAA STATS ---------------------------------------------------------
  echo "${1} ${total_ipaa_may} ${total_ipaa_no} ${total_naive_may_from_total}" >> ${ROOT}/ipAA.csv 


}

#---------------------------------------------------------------------------------------

run_mead() {

  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    #---- RUN MEAD -------------------------------------------------------------------------
    python ${PASHA_OBJ}/mead dfg.__offload_func_${ID}.dot __offload_func_${ID}.aa.txt
    mv mead.graph.stats.txt mead.graph.${n}.stats.txt
    #---------------------------------------------------------------------------------------
  done
}

#---------------------------------------------------------------------------------------

run_ldst_edges() {

  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    #---- RUN MEAD -------------------------------------------------------------------------
    python ${LDST_EDGES}/create_ldst_edges.py dfg.__offload_func_${ID}.dot 
    #---------------------------------------------------------------------------------------
  done
}



#---------------------------------------------------------------------------------------
#
# run_may_prof() {
#
#
#   #-----RUN MAY_PROF----------------------------------------------------------------------------------
#   mayedgefile=__offload_func_${ID}.may.txt
#   memfile=${TraceDir}/${x}/axc-dump.out.gz
#
#   for n in {0..4}; do 
#     $MAY_PROF_BIN -i $mayedgefile -m ${memfile} > may-prof-${n}.out 2> may-prof-${n}.err
#     ### # ------ to get stats from may-prof run ---------------------------------------
#     ### ensure you delete the first line of stat file
#     may_prof_stats  may-prof-${n}.out ${n} ${x} >>${ROOT}/oracle_may.csv
#     # ###----------------------------------------------------------------------------
#
#   done
# }


#---------------------------------------------------------------------------------------

run_may_prof_selected() {

  dir=$1
  pushd $DOTBASE/$dir > /dev/null
  # echo `pwd`

  #----------------------------------------
  # get PathId
  dfgfile=`ls dfg.__offload_func_*`
  # echo "dfgfile: $dfgfile"

  IFS='_' read -ra ADDR <<< "$dfgfile"
  for i in "${ADDR[@]}"; 
  do
    x=$i
  done

  pathid="${x%.*}"

  #----------------------------------------
  # get rankid

  pathfile=`ls path-seq*`
  EXTENSION=`echo "$pathfile" | cut -d'-' -f3`
  rankid=`echo "$EXTENSION" | cut -d'.' -f1`

  popd > /dev/null

  #-----RUN MAY_PROF----------------------------------------------------------------------------------
  ID=$pathid
  n=$rankid
  echo " ID: $ID n: $n"
  mayedgefile=__offload_func_${ID}.may.txt
  memfile=${TraceDir}/${dir}/axc-dump.out.gz
  # echo "$dir pathid: $pathid rankid: $rankid"

  # $MAY_PROF_BIN -i $mayedgefile -m ${memfile} > may-prof-${n}.out 2> may-prof-${n}.err
  ### # ------ to get stats from may-prof run ---------------------------------------
  may_prof_stats  may-prof-${n}.out ${n} ${dir} >>${ROOT}/oracle_may.csv
  # # ###----------------------------------------------------------------------------

}
#---------------------------------------------------------------------------------------






#---------------------------------------------------------------------------------------
get_dataflow_edges() {

  dir=$1
  pushd $DOTBASE/$dir > /dev/null
  # echo `pwd`

  #----------------------------------------
  # get PathId
  dfgfile=`ls dfg.__offload_func_*`
  # echo "dfgfile: $dfgfile"

  IFS='_' read -ra ADDR <<< "$dfgfile"
  for i in "${ADDR[@]}"; 
  do
    x=$i
  done

  pathid="${x%.*}"

  #----------------------------------------
  # get rankid

  pathfile=`ls path-seq*`
  EXTENSION=`echo "$pathfile" | cut -d'-' -f3`
  rankid=`echo "$EXTENSION" | cut -d'.' -f1`

  popd > /dev/null


  #---------------------------------
  # for n in {0..4}; do 
  n=$rankid
  s1=`egrep "num-dataflow-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
  s2=`egrep "num-added-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
  mead_edges=`echo "$s1+$s2" |bc -l`
  echo "$1 ${n} ${s1} ${mead_edges}" >>${ROOT}/num_dataflow_edges.csv
  # done

}
#----------------------------------------------------------------------------------------


get_offload_mem_dep() {

  dir=$1
  dotbase=$DOTBASE/${dir}
  pushd $dotbase > /dev/null

  #----------------------------------------
  # get PathId
  dfgfile=`ls dfg.__offload_func_*`
  # echo "dfgfile: $dfgfile"

  IFS='_' read -ra ADDR <<< "$dfgfile"
  for i in "${ADDR[@]}"; 
  do
    x=$i
  done

  pathid="${x%.*}"
  # echo "pathid: $pathid"

  #----------------------------------------
  # get rankid


  pathfile=`ls path-seq*`
  EXTENSION=`echo "$pathfile" | cut -d'-' -f3`
  rankid=`echo "$EXTENSION" | cut -d'.' -f1`

  # echo "rankid : $rankid"

  popd > /dev/null

  #---------------------------------------------------------------------------------------
  ID=$pathid
  n=$rankid
  echo " ID: $ID n:$n"
  echo -ne "${dir} " >> ${ROOT}/offload_stats.csv
  eval " python $HOTPATH_ALIAS   $TraceDir/${dir}/axc-dump.out.${n}.gz" >> ${ROOT}/offload_stats.csv 

}

#---------------------------------------------------------------------------------------

get_offload_mem_dep_5() {

  dir=$1

  for n in {0..4}; do 
    echo " app: {$dir} n:$n"
    # echo -ne "${dir} ${n} " >> ${ROOT}/offload_stats5.csv
    # eval " python $HOTPATH_ALIAS  $top5_TraceDir/${dir}/axc-dump.out.${n}.gz" >> ${ROOT}/offload_stats5.csv 

    eval " python $HOTPATH_ALIAS  $top5_TraceDir/${dir}/axc-dump.out.${n}.gz" > offload_stats.${n}.csv 
  done

}


grep_mem_dep_5() {

  dir=$1



  SS_no=0
  SS_yes=0
  LS_no=0
  LS_yes=0
  SL_no=0
  SL_yes=0


  if [ "${dir}" = "470.lbm" ]; then
    last=2
    echo "LAST=2"
    rank=('0' '1' '2' )
  else
    last=4
    rank=('0' '1' '2' '3' '4' )
  fi

     
  for n in "${rank[@]}";do 
  # for n in {0..4}; do 
    
    # eval "cat offload_stats.$n.csv "
    SS_no_conflict=$(awk '{print $2}' offload_stats.$n.csv)
    SS_yes_conflict=$(awk '{print $3}' offload_stats.$n.csv)
    LS_no_conflict=$(awk '{print $5}' offload_stats.$n.csv)
    LS_yes_conflict=$(awk '{print $6}' offload_stats.$n.csv)
    SL_no_conflict=$(awk '{print $8}' offload_stats.$n.csv)
    SL_yes_conflict=$(awk '{print $9}' offload_stats.$n.csv)

    # echo "Cal SS: $SS_no_conflict  $SS_yes_conflict LS: $LS_no_conflict $LS_yes_conflict SL: $SL_no_conflict $SL_yes_conflict"
    SS_no=`echo "$SS_no + $SS_no_conflict" | bc -l`
    SS_yes=`echo "$SS_yes + $SS_yes_conflict" | bc -l`

    LS_no=`echo "$LS_no + $LS_no_conflict" | bc -l`
    LS_yes=`echo "$LS_yes + $LS_yes_conflict" | bc -l`

    SL_no=`echo "$SL_no + $SL_no_conflict" | bc -l`
    SL_yes=`echo "$SL_yes + $SL_yes_conflict" | bc -l`

    # echo "Total SS: $SS_no $SS_yes  LS: $LS_no $LS_yes  SL: $SL_no $SL_yes"
    # echo "-------------------------------------------------------------"

  done
  echo -ne "$dir " >>$ROOT/offload_stats5.csv
  echo "$SS_no $SS_yes $LS_no $LS_yes $SL_no $SL_yes" >> $ROOT/offload_stats5.csv

}


