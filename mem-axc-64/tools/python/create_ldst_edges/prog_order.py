#!/usr/bin/python

import networkx as nx
import pprint 
import sys

pp = pprint.PrettyPrinter(indent=2)


def write_dfg(dfgfile, dfg):
    nx.drawing.nx_agraph.write_dot(dfg, 'ldst.'+dfgfile)


def add_edges(a,b, sorted_memops,dfg):
    for val in sorted_memops[a+1:b+1]:
        # print "adding edges :  val_a= " + str(sorted_memops[a]) + " val_b= " + str(val)
        dfg.add_edge(sorted_memops[a],val, color='orange')


def main(dfgfile):
    global pp
    dfg = nx.DiGraph(nx.drawing.nx_agraph.read_dot(dfgfile))
    
    memops = []
    # Create a list of memops in sorted order
    # x3 = dfg.nodes()
    for x1 in dfg.nodes():
        if dfg.node[x1]['opcode'] == 'Load' or dfg.node[x1]['opcode'] == 'Store':
            if '!LO' not in dfg.node[x1]['ir']:
                memops.append(int(x1))

    sorted_memops = sorted(memops)
    print sorted_memops
    a=0    

    for idx,val in enumerate(sorted_memops):
        if dfg.node[str(val)]['opcode']=='Store':
            b=idx
            print "a= " + str(a) + " b= " + str(b) 
            # print "a= " + str(a) +" val_a: "+ str(sorted_memops[a]) + " b= " + str(b) + " val_b: " + str(sorted_memops[b])
            add_edges(a,b,sorted_memops,dfg)
            a=b




    write_dfg(dfgfile,dfg)



if __name__ == "__main__":
    main(sys.argv[1])
