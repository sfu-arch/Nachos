#!/usr/bin/python

import networkx as nx
import pprint 
import sys
from math import log

pp = pprint.PrettyPrinter(indent=2)



def write_dfg(dfgfile, dfg):
    nx.drawing.nx_agraph.write_dot(dfg, 'mead.'+dfgfile)


def main(dfgfile):
    hist = {0:0, 1:0, 2:0, 3:0}
    global pp
    dfg = nx.DiGraph(nx.drawing.nx_agraph.read_dot(dfgfile))
    assert nx.algorithms.is_directed_acyclic_graph(dfg) 
    nodes=[d for d in dfg.nodes_iter(data=False)]

    CDE=0
    num_memop=0

    for n in dfg.nodes():
        if "!LO" in dfg.node[n]['ir']:
            continue
        # node is a memop
        if dfg.node[n]['opcode'] == "Load" or \
                dfg.node[n]['opcode'] == "Store": 
            num_memop +=1
            idx =0
            for src, tgt in dfg.in_edges(n):
                src_opcode = str(dfg.node[src]['opcode'])

                #Src op should not be !LO
                if( src_opcode == "Load" or src_opcode == "Store"):
                    CDE +=1
                    idx +=1

            if idx>2:
                hist[3] +=1
            else:
                hist[idx] +=1
            
            print "nodeid:" + str(n) + "fanin: " + str(idx) 

    for key, value in hist.items():
        print "bin " + str(key) + " " + str(value) , 
        sys.stderr.write(str(value) + " ")


    sys.stderr.write(str(num_memop) + " ")
    print " "
    print "orig_histogram :" 
    print hist
    print "CDE: " + str(CDE)
    print "num_memops: " + str(num_memop)


if __name__ == "__main__":
    main(sys.argv[1])
