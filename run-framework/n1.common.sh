#---------------------------------------------------------------------------------------
get_selected_pathid_rankid() {

  #Updated for partial
  dir2=$1
  pushd $DOTBASE/$dir2 > /dev/null
  # echo `pwd`

  #----------------------------------------
  # get PathId
  dfgfile=`ls dfg.__offload_func_*`
  # echo "dfgfile: $dfgfile"

  IFS='_' read -ra ADDR <<< "$dfgfile"
  # Just to fix color in script
  #`

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
}


#---------------------------------------------------------------------------------------

grep_edges() {

  dir=$1
  get_selected_pathid_rankid $dir
  n=$rankid
  ID=$pathid

  #Updated for partial
    #---- MEAD STATS ---------------------------------------------------------
    if [ -f "mead.graph.${n}.stats.txt" ]; then
      #per iteration stats
      s1=`egrep "num-orig-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`
      s2=`egrep "num-added-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`
      s3=`egrep "num-dataflow-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`

      # echo "app num-orig-edges num-added-edges num-dataflow-edges">$ROOT/selected_path_edges.csv 
      echo "$dir $s1 $s2 $s3" >>$ROOT/selected_path_edges.csv
    fi
}




#----------------------------------------------------------------------------------------
may_prof_stats () {
  #Updated for partial

  no_count=`cut -d' ' -f 4  $1 | grep -v "^0$" | wc -l`
  yes_count=`cut -d' ' -f 5  $1 | grep -v "^0$" | wc -l`

# echo "toal_no_conflict_edges_are_zero: ${MUSTZ_count} total_must_conflicts_edges_are_zero: ${NOZ_count} total_SPLIT_cases: ${NZ_count} "

b=`egrep "num-no-alias" aew.$2.stats.txt | cut -d' ' -f2`
c=`egrep "num-must-alias" aew.$2.stats.txt | cut -d' ' -f2`
h=`egrep "num-ipaa-no-alias" aew.$2.stats.txt | cut -d' ' -f2`


no_conflict=`echo "$b + $h + $no_count" |bc -l`
yes_conflict=`echo "$c + $yes_count" |bc -l`
# echo "$3 $2 $no_conflict $yes_conflict $NZ_count"
echo "$3 $no_conflict $yes_conflict"
}




#----------------------------------------------------------------------------------------
run_longest_path() {
  #./longest_path.py dfg.__offload_func_862567.dot latency.csv 179.art
  #Updated for partial

  bench=$1
  # bench=179.art
  dotbase=$DOTBASE/${bench}
  pushd $dotbase > /dev/null
  dot_dfg=`ls dfg.*`
  dot_ldstd=`ls ldst.dfg.*`
  dot_mead=`ls mead.dfg.*`
  # echo $dotfile
  # echo $dotfile2
  popd >/dev/null

  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    # bindir=/home/nvedula/summer2016/mem-axc-64/tools/python/longest_path
    # $bindir  /longest_path.py $dotbase/$dotfile $bindir/latency.csv $bench >${tool}.longest_path_${ID}.stats.txt __offload_func_${ID}.aa.txt 
    python $LONGEST_PATH_CONST_LAT $dotbase/$dot_dfg  > dfg.longest_path_${n}.stats.txt
  done

  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    # bindir=/home/nvedula/summer2016/mem-axc-64/tools/python/longest_path
    # $bindir/longest_path.py $dotbase/$dotfile2 $bindir/latency.csv $bench >${tool}.longest_path_${ID}.stats.txt __offload_func_${ID}.aa.txt 
    python $LONGEST_PATH_CONST_LAT $dotbase/$dot_mead  __offload_func_${ID}.aa.txt > mead.longest_path_${n}.stats.txt
  done
}
#---------------------------------------------------------------------------------------
grep_longest_path() {

  dir=$1 
  get_selected_pathid_rankid $dir

  n=$rankid
  echo $dir ID= $pathid n= $rankid

  #Updated for partial
  dfg=dfg.longest_path_${n}.stats.txt
  mead=mead.longest_path_${n}.stats.txt

  dfg_crit_length=`egrep "opcode_length" $dfg | cut -d' ' -f2`
  dfg_crit_latency=`egrep "total_cycles" $dfg | cut -d' ' -f2`
  mead_crit_length=`egrep "opcode_length" $mead | cut -d' ' -f2`
  mead_crit_latency=`egrep "total_cycles" $mead | cut -d' ' -f2`

  cde=`egrep -c "CDE" $mead`

  echo "$dir $dfg_crit_length $mead_crit_length $dfg_crit_latency $mead_crit_latency $cde">>$ROOT/critical_length.csv
  # echo "$dir $dfg_crit_latency $mead_crit_latency">>$ROOT/critical_length.csv
}

 

#---------------------------------------------------------------------------------------
grep_all_AAstats() {

  #Updated for partial
  #$1 = x : application name

  #Cumm. Global Vars
  #MEAD
  total_orig_edges=0
  total_added_edges=0

  #NAIVE AA
  total_naive_may=0
  total_naive_no=0
  total_naive_must=0
  total_naive_partial=0
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
      # s3=`egrep "num-dataflow-edges" mead.graph.${n}.stats.txt | cut -d' ' -f2`

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
    naive_must=${c}
    naive_no=${b}
    naive_may=`echo "$e" | bc -l`
    naive_partial=`echo "$d" | bc -l`
    naive_total=`echo "$naive_no + $naive_may + $naive_must + $naive_partial" | bc -l`
    naive_may_from_total=0

    # Cummulative stats for all top 5 paths
    total_naive_may=$((total_naive_may + naive_may))
    total_naive_no=$((total_naive_no + naive_no))
    total_naive_must=$((total_naive_must + naive_must))
    total_naive_partial=$((total_naive_partial + naive_partial))
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
  # total_naive_may_from_total=0
  # if [  ${total_naive_total} -ne 0  ]; then
  #
  #   total_naive_fraction=`echo "$total_naive_may/$total_naive_total*100" | bc -l`
  #   total_naive_may_from_total=${total_naive_fraction}
  # fi

  NAIVE_MAY=`echo "$total_naive_may + $total_naive_partial" | bc -l`
  echo "${1} ${NAIVE_MAY} ${total_naive_no} ${total_naive_must}" >> ${ROOT}/naiveAA.csv 


  #---- IPAA STATS ---------------------------------------------------------
  IPAA_MAY=`echo "$total_ipaa_may + $total_naive_partial" | bc -l`
  total_naive_edges=`echo " $NAIVE_MAY + $total_naive_must" | bc -l`
  # total_ipaa_edges=`echo " $total_ipaa_may + $total_naive_must" | bc -l`
  echo "${1} ${total_naive_total} ${IPAA_MAY} ${total_naive_must}" >> ${ROOT}/ipAA.csv 
  
  
  # echo "${1} ${total_ipaa_may} ${total_ipaa_no} ${total_naive_may_from_total}" >> ${ROOT}/ipAA.csv 


}

#---------------------------------------------------------------------------------------

run_mead() {

  #Updated for partial
  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`

    #---- RUN TARTAN -------------------------------------------------------------------------
    python ${PASHA_OBJ}/mead dfg.__offload_func_${ID}.dot __offload_func_${ID}.naiveaa.txt tartan
    mv tartan.graph.stats.txt tartan.graph.${n}.stats.txt
    #---------------------------------------------------------------------------------------
    #---- RUN MEAD -------------------------------------------------------------------------
    python ${PASHA_OBJ}/mead dfg.__offload_func_${ID}.dot __offload_func_${ID}.aa.txt mead
    mv mead.graph.stats.txt mead.graph.${n}.stats.txt
    #---------------------------------------------------------------------------------------


  done
}

#---------------------------------------------------------------------------------------

add_tartan_prefix_alias_files() {

  ALIAS_FILES=`ls __offload*.aa.txt __offload*.may.txt __offload*.must.txt __offload*.naiveaa.txt`

  for x in ${ALIAS_FILES[@]}; do
    mv $x tartan.$x
  done
}

#---------------------------------------------------------------------------------------

verify_diff_tartan_nachos_alias() {

  ALIAS_FILES=`ls __offload*.aa.txt` # __offload*.may.txt __offload*.must.txt __offload*.naiveaa.txt`

  for x in ${ALIAS_FILES[@]}; do
    echo "$1 $x" >> $ROOT/diff_alias.txt
    diff $x tartan.$x >> $ROOT/diff_alias.txt
  done
}
#---------------------------------------------------------------------------------------

run_midas() {

  #Updated for partial
  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    #---- RUN MEAD -------------------------------------------------------------------------
    python /home/nvedula/summer2016/mem-axc-64/tools/python/mead/midas.py  dfg.__offload_func_${ID}.dot __offload_func_${ID}.aa.txt  __offload_func_${ID}.must.txt
    mv mead.graph.stats.txt mead.graph.${n}.stats.txt
    #---------------------------------------------------------------------------------------
  done
}
#---------------------------------------------------------------------------------------

run_ldst_edges() {

  #Updated for partial
  for n in {0..4}; do 
    ID=`head -n 1 path-seq-${n}.txt  | cut -d' ' -f1`
    #---- RUN MEAD -------------------------------------------------------------------------
    python ${LDST_EDGES}/create_ldst_edges.py dfg.__offload_func_${ID}.dot 
    #---------------------------------------------------------------------------------------
  done
}




#---------------------------------------------------------------------------------------
get_dataflow_edges() {

  #Updated for partial
  dir=$1

  get_selected_pathid_rankid $dir
  #---------------------------------
  # for n in {0..4}; do 
  n=$rankid
  echo $dir ID= $pathid n= $rankid
  s1=`egrep "num-dataflow-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
  s2=`egrep "num-added-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
  mead_edges=`echo "$s1+$s2" |bc -l`
  echo "$1 ${n} ${s1} ${mead_edges}" >>${ROOT}/num_dataflow_edges.csv
  # done

}

#---------------------------------------------------------------------------------------
get_alias_info_selected_path() {
  #Updated for partial

  dir=$1
  get_selected_pathid_rankid $dir
  #---------------------------------
  n=$rankid
  echo $dir ID= $pathid n= $rankid
  tmp_simplified=`egrep "num-added-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
  tmp_simplified_may=`egrep "num-added-may-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
  simplified_must=`egrep "num-added-must-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`

  ipaa_total=`egrep "num-aa-pairs"        aew.$n.stats.txt | cut -d' ' -f2`
  tmp_naive_may=`egrep  "num-may-alias-naive" aew.$n.stats.txt | cut -d' ' -f2`
  ipaa_may=`egrep   "num-ipaa-may-alias"  aew.$n.stats.txt | cut -d' ' -f2`
  must=`egrep       "num-must-alias"      aew.$n.stats.txt | cut -d' ' -f2`
  partial=`egrep    "num-partial-alias"   aew.$n.stats.txt | cut -d' ' -f2`

  # XXX this is a hack -- gives correct result in this case
  # ipaa-naive also includes partial and any result using ipaa should be carefully observed
  naive_may=`echo "$tmp_naive_may + $partial" |bc -l`


  # Accounting for Polly Benchmarks : 183.equake 470.lbm 444.namd bodytrack dwt53
  # Make all simplified may edges = 0

  out=`echo $dir`
  if [ "${out}" == "183.equake" ]; then
    simplified_may=0
    simplified=`echo "$tmp_simplified - $tmp_simplified_may" | bc -l`
    >&2 echo "$_apps : is pollybench making simplified_may_edges zero" 

  elif [ "${out}" == "470.lbm" ]; then
    simplified_may=0
    simplified=`echo "$tmp_simplified - $tmp_simplified_may" | bc -l`
    >&2 echo "$_apps : is pollybench making simplified_may_edges zero" 

  elif [ "${out}" == "444.namd" ]; then
    simplified_may=0
    simplified=`echo "$tmp_simplified - $tmp_simplified_may" | bc -l`
    >&2 echo "$_apps : is pollybench making simplified_may_edges zero" 

  elif [ "${out}" == "bodytrack" ]; then
    simplified_may=0
    simplified=`echo "$tmp_simplified - $tmp_simplified_may" | bc -l`
    >&2 echo "$_apps : is pollybench making simplified_may_edges zero" 

  elif [ "${out}" == "dwt53" ]; then
    simplified_may=0
    simplified=`echo "$tmp_simplified - $tmp_simplified_may" | bc -l`
    >&2 echo "$_apps : is pollybench making simplified_may_edges zero" 
  else
    simplified_may=$tmp_simplified_may
    simplified=$tmp_simplified

  fi





  total_naive_aa_pairs=`echo " $ipaa_total - $ipaa_may + $naive_may" | bc -l`
  total_naive_must=$must 
  total_naive_edges=`echo "$naive_may + $total_naive_must"  | bc -l`

  if [ "${total_naive_edges}" == "0" ]; then
    total_naive_may_percent=0
  else
    total_naive_may_percent=`echo "scale=3; $naive_may/$total_naive_edges*100.0" | bc -l`
  fi

  if [ "${simplified}" == "0" ]; then
    total_simplified_may_percent=0
  else
    total_simplified_may_percent=`echo "scale=3; $simplified_may/$simplified*100.0" | bc -l`
  fi



  >&2 echo "$dir ${total_naive_aa_pairs} ${naive_may} ${total_naive_must} ${simplified} $simplified_may $simplified_must "

  echo "$dir ${total_naive_aa_pairs} ${naive_may} ${total_naive_must} ${simplified} $simplified_may $simplified_must" >>${ROOT}/selected_path_alias_info.csv
  echo "$dir ${total_naive_aa_pairs} ${total_naive_edges} ${simplified} $total_naive_may_percent $total_simplified_may_percent" >>${ROOT}/plot_selected_path_alias_info.csv

}


#----------------------------------------------------------------------------------------
get_alias_info_top5_path() {
  #Updated for partial

  dir=$1
  #---------------------------------
  
  SIMP_TOTAL=0
  SIMP_MAY=0
  SIMP_MUST=0
  
  IPAA_TOTAL=0
  IPAA_MAY=0

  NAIVE_TOTAL=0
  NAIVE_MAY=0
  NAIVE_MUST=0
  NAIVE_PARTIAL=0

  out=`echo $dir`
  if [ "${out}" == "470.lbm" ]; then
    max=3  
  else
    max=5
  fi

  echo $out $max

  for ((n = 0 ; n < max ; n++ ));  
  do
    simplified=`egrep "num-added-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
    simplified_may=`egrep "num-added-may-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`
    simplified_must=`egrep "num-added-must-edges" mead.graph.$n.stats.txt | cut -d' ' -f2`

    # ipaa_may=`egrep   "num-ipaa-may-alias"  aew.$n.stats.txt | cut -d' ' -f2`

    naive_no=`egrep "num-no-alias" aew.${n}.stats.txt | cut -d' ' -f2`
    naive_may=`egrep  "num-may-alias-naive" aew.$n.stats.txt | cut -d' ' -f2`
    naive_must=`egrep       "num-must-alias"      aew.$n.stats.txt | cut -d' ' -f2`
    naive_partial=`egrep    "num-partial-alias"   aew.$n.stats.txt | cut -d' ' -f2`
    naive_total=`echo "$naive_no + $naive_may + $naive_must + $naive_partial" | bc -l`



    # Loop Addition -----------------------------------------------

    SIMP_TOTAL=`echo "$SIMP_TOTAL + $simplified" | bc -l`
    SIMP_MAY=`echo "$SIMP_MAY + $simplified_may" | bc -l`
    SIMP_MUST=`echo "$SIMP_MUST + $simplified_must" | bc -l`

    # IPAA_MAY=`echo "$IPAA_MAY + $ipaa_may" | bc -l`
    
    
    # NAIVE_MAY=`echo "$NAIVE_MAY + $naive_may" | bc -l`
    # NAIVE_MUST=`echo "$NAIVE_MUST + $naive_must" | bc -l`
    # NAIVE_PARTIAL=`echo "$NAIVE_PARTIAL + $naive_partial" | bc -l`
    NAIVE_TOTAL=$(( $NAIVE_TOTAL + $naive_total))
    
  done

  # total_naive_edges=`echo "$NAIVE_MAY + $NAIVE_MUST + $NAIVE_PARTIAL"  | bc -l`
  total_simp_edges=`echo "$SIMP_MAY + $SIMP_MUST"  | bc -l`

  # >&2 echo "$dir ${total_naive_edges} $SIMP_MAY $SIMP_MUST $total_simp_edges"
  echo "$dir $NAIVE_TOTAL $SIMP_MAY $SIMP_MUST $total_simp_edges" >>${ROOT}/top5_alias_info.csv

}


#----------------------------------------------------------------------------------------


get_offload_mem_dep() {
  #Updated for partial

  dir=$1
  dotbase=$DOTBASE/${dir}
  pushd $dotbase > /dev/null

  #----------------------------------------
  # get PathId
  dfgfile=`ls dfg.__offload_func_*`
  # echo "dfgfile: $dfgfile"

  IFS='_' read -ra ADDR <<< "$dfgfile"
  #`

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

  #Updated for partial
  dir=$1

  for n in {0..4}; do 
    echo " app: {$dir} n:$n"
    # echo -ne "${dir} ${n} " >> ${ROOT}/offload_stats5.csv
    # eval " python $HOTPATH_ALIAS  $top5_TraceDir/${dir}/axc-dump.out.${n}.gz" >> ${ROOT}/offload_stats5.csv 

    eval " python $HOTPATH_ALIAS  $top5_TraceDir/${dir}/axc-dump.out.${n}.gz" > offload_stats.${n}.csv 
  done

}


grep_mem_dep_5() {
  #Updated for partial

  dir=$1



  SS_no=0
  SS_yes=0
  LS_no=0
  LS_yes=0
  SL_no=0
  SL_yes=0
  LL_no=0
  LL_yes=0




  out=`echo $dir`
  if [ "${out}" == "470.lbm" ]; then
    max=3   
  else
    max=5
  fi
  echo $out $max

  for ((n = 0 ; n < max ; n++ )); do 
    
    # eval "cat offload_stats.$n.csv "
    SS_no_conflict=$(awk '{print $2}' offload_stats.$n.csv)
    SS_yes_conflict=$(awk '{print $3}' offload_stats.$n.csv)
    LS_no_conflict=$(awk '{print $5}' offload_stats.$n.csv)
    LS_yes_conflict=$(awk '{print $6}' offload_stats.$n.csv)
    SL_no_conflict=$(awk '{print $8}' offload_stats.$n.csv)
    SL_yes_conflict=$(awk '{print $9}' offload_stats.$n.csv)

    LL_no_conflict=$(awk '{print $11}' offload_stats.$n.csv)
    LL_yes_conflict=$(awk '{print $12}' offload_stats.$n.csv)

    # echo "Cal SS: $SS_no_conflict  $SS_yes_conflict LS: $LS_no_conflict $LS_yes_conflict SL: $SL_no_conflict $SL_yes_conflict"
    SS_no=`echo "$SS_no + $SS_no_conflict" | bc -l`
    SS_yes=`echo "$SS_yes + $SS_yes_conflict" | bc -l`

    LS_no=`echo "$LS_no + $LS_no_conflict" | bc -l`
    LS_yes=`echo "$LS_yes + $LS_yes_conflict" | bc -l`

    SL_no=`echo "$SL_no + $SL_no_conflict" | bc -l`
    SL_yes=`echo "$SL_yes + $SL_yes_conflict" | bc -l`


    LL_no=`echo "$LL_no + $LL_no_conflict" | bc -l`
    LL_yes=`echo "$LL_yes + $LL_yes_conflict" | bc -l`



    # echo "Total SS: $SS_no $SS_yes  LS: $LS_no $LS_yes  SL: $SL_no $SL_yes"
    # echo "-------------------------------------------------------------"

  done
  total_noAA=`echo "$SS_no + $LS_no + $SL_no" | bc -l`
  total_yesAA=`echo "$SS_yes + $LS_yes + $SL_yes" | bc -l`

  echo -ne "$dir " >>$ROOT/offload_stats5.csv
  echo "$SS_no $SS_yes $LS_no $LS_yes $SL_no $SL_yes $LL_no $LL_yes" >> $ROOT/offload_stats5.csv
  echo "$dir $total_noAA $total_yesAA" >> $ROOT/oracle_conflict.csv

}

grep_mem_dep_1() {

  #Updated for partial
  dir=$1



  SS_no=0
  SS_yes=0
  LS_no=0
  LS_yes=0
  SL_no=0
  SL_yes=0
  LL_no=0
  LL_yes=0


  out=`echo $dir`
  get_selected_pathid_rankid $dir
  ID=$pathid n=$rankid
  echo "ID= $ID rankid= $n"

  # eval "cat offload_stats.$n.csv "
  SS_no_conflict=$(awk '{print $2}' offload_stats.$n.csv)
  SS_yes_conflict=$(awk '{print $3}' offload_stats.$n.csv)
  LS_no_conflict=$(awk '{print $5}' offload_stats.$n.csv)
  LS_yes_conflict=$(awk '{print $6}' offload_stats.$n.csv)
  SL_no_conflict=$(awk '{print $8}' offload_stats.$n.csv)
  SL_yes_conflict=$(awk '{print $9}' offload_stats.$n.csv)
  LL_no_conflict=$(awk '{print $11}' offload_stats.$n.csv)
  LL_yes_conflict=$(awk '{print $12}' offload_stats.$n.csv)



  echo "Cal SS: $SS_no_conflict  $SS_yes_conflict LS: $LS_no_conflict $LS_yes_conflict SL: $SL_no_conflict $SL_yes_conflict LL: $LL_yes_conflict $LL_no_conflict"
  SS_no=`echo "$SS_no + $SS_no_conflict" | bc -l`
  SS_yes=`echo "$SS_yes + $SS_yes_conflict" | bc -l`

  LS_no=`echo "$LS_no + $LS_no_conflict" | bc -l`
  LS_yes=`echo "$LS_yes + $LS_yes_conflict" | bc -l`

  SL_no=`echo "$SL_no + $SL_no_conflict" | bc -l`
  SL_yes=`echo "$SL_yes + $SL_yes_conflict" | bc -l`

  LL_no=`echo "$LL_no + $LL_no_conflict" | bc -l`
  LL_yes=`echo "$LL_yes + $LL_yes_conflict" | bc -l`


  # echo "Total SS: $SS_no $SS_yes  LS: $LS_no $LS_yes  SL: $SL_no $SL_yes"
  # echo "-------------------------------------------------------------"

  echo -ne "$dir " >>$ROOT/offload_stats1.csv
  echo "$SS_no $SS_yes $LS_no $LS_yes $SL_no $SL_yes $LL_no $LL_yes" >> $ROOT/offload_stats1.csv

}




get_aew_summary() {

  #Updated for partial
  for k in {0..4}; do
    echo "-----------------------" >> $ROOT/aew.stats.txt
    echo $1  aew.$k.stats.txt >>$ROOT/aew.stats.txt
    echo "-----------------------" >>$ROOT/aew.stats.txt
    cat aew.$k.stats.txt >>$ROOT/aew.stats.txt
  done

}

get_ops() {

  #Updated for partial
  dir=$1
  pushd $DOTBASE/$dir > /dev/null
  # echo `pwd`

  #----------------------------------------
  # get PathId
  dfgfile=`ls dfg.__offload_func_*`
  # echo "dfgfile: $dfgfile"

  IFS='_' read -ra ADDR <<< "$dfgfile"
  #`

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
  search_file=__offload_func_$ID.stats.txt
  Ld=`grep "Load"   $search_file | awk '{print $2}'`
  St=`grep "Store"  $search_file | awk '{print $2}'`

  intop=`egrep "^AShr|^Add|^And|^Xor|^Mul|^Or|^SDiv|^Shl|^SRem|^SExt|^Sub|^LShr|^UDiv|^URem" $search_file | awk 'BEGIN{sum=0} {sum+=$2} END{print sum}'`
  fpop=`egrep "^FAdd|^FCmp|^GetElementPtr|^FDiv|^FMul|^FRem|^FSub" $search_file | awk 'BEGIN{sum=0} {sum+=$2} END{print sum}'`
  totalop=`echo "$intop + $fpop + $Ld + $St" | bc -l`
  echo "$dir $Ld $St $intop $fpop $totalop" >>$ROOT/ops_distribution.csv

}


run_fanins() {
  dir=$1
  get_selected_pathid_rankid $dir
  n=$rankid
  ID=$pathid
  $FANIN mead.dfg.__offload_func_${ID}.dot > fanin.$n.txt
}


grep_fanins() {
  dir=$1
  get_selected_pathid_rankid $dir
  n=$rankid
  ID=$pathid

  echo -ne "$dir ">>$ROOT/fanin_summary.txt
  grep "bin" fanin.$n.txt >> $ROOT/fanin_summary.txt
}

grep_walltime() {
  dir=$1
  get_selected_pathid_rankid $dir
  n=$rankid
  ID=$pathid

  echo "-- $dir --">>$ROOT/walltime.csv
  grep "AliasEdgeWriter" mwe.$n.log >>$ROOT/walltime.csv
}












#---------------------------------------------------------------------------------------
# We are not using may-prof tool anymore for ISCA2017
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
#
#
# #---------------------------------------------------------------------------------------
#
# run_may_prof_selected() {
#
#   dir=$1
#   get_selected_pathid_rankid $dir
#   echo $PWD
#   #-----RUN MAY_PROF----------------------------------------------------------------------------------
#   ID=$pathid
#   n=$rankid
#   echo " ID: $ID n: $n"
#   mayedgefile=__offload_func_${ID}.may.txt
#   memfile=${TraceDir}/${dir}/axc-dump.out.gz
#   echo "$dir pathid: $pathid rankid: $rankid"
#
#   $MAY_PROF_BIN -i $mayedgefile -m ${memfile} > may-prof-${n}.out 2> may-prof-${n}.err
#   # # ###----------------------------------------------------------------------------
#
# }
# #---------------------------------------------------------------------------------------
#
# grep_may_prof_stats() {
#
#   dir=$1
#   #-----RUN MAY_PROF----------------------------------------------------------------------------------
#
#   get_selected_pathid_rankid $dir
#
#   echo `pwd`
#   ID=$pathid
#   n=$rankid
#   echo " ID: $ID n: $n"
#   mayedgefile=__offload_func_${ID}.may.txt
#   memfile=${TraceDir}/${dir}/axc-dump.out.gz
#   echo "$dir pathid: $pathid rankid: $rankid"
#
#   ### # ------ to get stats from may-prof run ---------------------------------------
#   may_prof_stats  may-prof-${n}.out ${n} ${dir} >>${ROOT}/oracle_may.csv
#   # # ###----------------------------------------------------------------------------
#
# }



