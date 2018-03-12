#!/usr/bin/python

import networkx as nx
import pprint 
import sys
import os

pp = pprint.PrettyPrinter(indent=2)
data = {}
data['num-added-edges'] = 0
data['num-added-may-edges'] = 0
data['num-added-must-edges'] = 0
data['num-orig-edges'] = 0

def add_edges(dfg, aa_edges, ismust):
    global pp
    for (src,dst) in aa_edges:
        if dst not in nx.dfs_postorder_nodes(dfg, src):
            dfg.add_edge(src, dst, {'color' : 'red'})
            if(ismust):
                data['num-added-must-edges'] += 1 
            else:
                data['num-added-may-edges'] +=1

            data['num-added-edges'] +=1


def write_dfg(dfgfile, dfg, outfile_prefix):
    nx.drawing.nx_agraph.write_dot(dfg, outfile_prefix +'.'+dfgfile)


def main(dfgfile, aafile, mustfile, outfile_prefix):
    global pp
    dfg = nx.DiGraph(nx.drawing.nx_agraph.read_dot(dfgfile))
    assert nx.algorithms.is_directed_acyclic_graph(dfg) 
    data['num-dataflow-edges'] = nx.number_of_edges(dfg)
    
    # # use may file
    aa = set() 
    with open(mustfile, 'r') as f:
        for l in f:
            s = l.strip().split()
            aa.add((s[0],s[1]))
    # data['num-orig-edges'] = len(aa)
    add_edges(dfg, aa, True)

    # Using aafile which contains may and must edges
    # but since, must edges have already been added
    # all the newly added edges will be must edges only
    aa = set() 
    if os.stat(aafile).st_size != 0:
        with open(aafile, 'r') as f:
            for l in f:
                s = l.strip().split()
                aa.add((s[0],s[1]))
        data['num-orig-edges'] = len(aa)
        add_edges(dfg, aa, False)

    # use may file
    # aa = set() 
    # with open(mayfile, 'r') as f:
    #     for l in f:
    #         s = l.strip().split()
    #         aa.add((s[0],s[1]))
    # # data['num-orig-edges'] = len(aa)
    # add_edges(dfg, aa, True)





    write_dfg(dfgfile, dfg, outfile_prefix)

def dump_stats( outfile_prefix):
    global data
    with open(outfile_prefix + '.graph.stats.txt', 'w') as f:
        for k,v in data.iteritems():
            f.write(' '.join([k,str(v),'\n']))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    dump_stats( sys.argv[4])
