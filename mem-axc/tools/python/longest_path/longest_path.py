#!/usr/bin/python

import networkx as nx
import pprint 
import sys
import math

pp = pprint.PrettyPrinter(indent=2)
data = {}



def dump_stats(dfg,A, aafile):
    #rint(A)  
    # with open('longest.graph.stats.txt', 'w') as f:
    CDEs = set() 
    with open(aafile, 'r') as f:
        for line in f:
            s = line.strip().split(' ')
            assert len(s) == 2
            CDEs.add((s[0],s[1]))

    # Classify edges in A as DDE / CDE
    # Conflict Dependency Edge and Data Dependency Edge
    edges = ['-']
    for n in range(1, len(A)):
        if (A[n-1],A[n]) in CDEs:
            edges.append('CDE')
        else:
            edges.append('---')
        

    hist ={}
    opcodelist = []
    for x in A:
        opcode = str(dfg.node[x]['opcode'])
        opcodelist.append(opcode)
        if opcode not in hist.keys():
            hist[opcode] = 0
        hist[opcode] += 1

    # [print(x)  for x in opcodelist]
    print( "opcode_length " + str(len(opcodelist)) )
    tot_cycles=0
    for nodes in A:
        tot_cycles = tot_cycles+dfg.node[nodes]['weight']

    print( "total_cycles " + str(tot_cycles) )

    for key, val  in hist.iteritems():
        print( str(key)+" " +str(val))

    print "-------"
    for x,y,z in zip(A, opcodelist, edges):
        print str(x), str(y), str(z)
    print( "\n")


#--------------------------------------------------------------------------------
def add_weight(dfg,mem_latency):
    lat = int(math.floor(float(mem_latency)))
    for src in dfg.nodes():
        if dfg.node[src]['opcode'] == 'Load':
            dfg.node[src]['weight']= lat 
        elif dfg.node[src]['opcode'] == 'Store':
            dfg.node[src]['weight']= lat 
        elif dfg.node[src]['opcode'] == 'FDiv':
            dfg.node[src]['weight']=20
        elif dfg.node[src]['opcode'] == 'UDiv':
            dfg.node[src]['weight']=15
        elif dfg.node[src]['opcode'] == 'SDiv':
            dfg.node[src]['weight']=15
        elif dfg.node[src]['opcode'] == 'FMul':
            dfg.node[src]['weight']=7
        elif dfg.node[src]['opcode'] == 'Mul':
            dfg.node[src]['weight']=4
        # elif dfg.node[src]['opcode'] == 'GetElementPtr':
        #     dfg.node[src]['weight']=600
        else:
            dfg.node[src]['weight']=1
    return dfg


#--------------------------------------------------------------------------------
def read_latency(latfile):
    latency_dict = {} 
    with open(latfile, 'r') as f:
        for l in f:
            s = l.strip().split(',')
            # latency_dict.add((s[0],s[1]))
            latency_dict[s[0]] = s[1]
    return latency_dict



#--------------------------------------------------------------------------------

def longest_path(G):
    dist = {} # stores [node, distance] pair
    for node in nx.topological_sort(G):
        # pairs of dist,node for all incoming edges
        pairs = [(dist[v][0]+G.node[node]['weight'],v) for v in G.pred[node]] 
        if pairs:
            dist[node] = max(pairs)
        else:
            dist[node] = (0, node)
    node,(length,_)  = max(dist.items(), key=lambda x:x[1])
    path = []
    while length > 0:
        path.append(node)
        length,node = dist[node]
    return list(reversed(path)) 
#--------------------------------------------------------------------------------
def main(dfgfile, latencyfile, bench, aafile):
    global pp
    dfg = nx.DiGraph(nx.drawing.nx_agraph.read_dot(dfgfile))
    assert nx.algorithms.is_directed_acyclic_graph(dfg) 
    latency_dict= read_latency(latencyfile)
    mem_latency=latency_dict[bench]
    dfg = add_weight(dfg,mem_latency)
    A = longest_path(dfg)
    dump_stats(dfg,A, aafile)

    
#--------------------------------------------------------------------------------

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])




