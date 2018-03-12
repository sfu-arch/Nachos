#!/usr/bin/python

import networkx as nx
import pprint 
import sys

pp = pprint.PrettyPrinter(indent=2)


def write_dfg(dfgfile, dfg):
    nx.drawing.nx_agraph.write_dot(dfg, 'ldst.'+dfgfile)


def add_edges(a,b, sorted_memops,dfg):
    for val in sorted_memops[a+1:b]:
        # print "adding edges :  val_a= " + str(sorted_memops[a]) + " val_b= " + str(val)
        k = {}
        pred_set = nx.ancestors(dfg,str(val) )
        print pred_set
        if str(sorted_memops[a]) not in pred_set:
            print "adding edge " + str(sorted_memops[a]) + "-> " + str(val)
            dfg.add_edge(sorted_memops[a],val, color='orange')
        else:
            print str(sorted_memops[a]) + " is  a pred of " + str(val)


    for val in sorted_memops[a+1:b]:
            # print "adding edges :  val_a= " + str(sorted_memops[a]) + " val_b= " + str(val)
            k = {}
            pred_set = nx.ancestors(dfg,str(sorted_memops[b]) )
            print pred_set
            if str(val) not in pred_set:
                print "adding edge " + str(val) + "-> " + str(sorted_memops[b])
                dfg.add_edge(val,sorted_memops[b], color='orange')
            else:
                print str(sorted_memops[a]) + " is  a pred of " + str(val)

    #consequent store opertions
    if a+1 ==b:
        for val in sorted_memops[a+1:b+1]:
            # print "adding edges :  val_a= " + str(sorted_memops[a]) + " val_b= " + str(val)
            k = {}
            pred_set = nx.ancestors(dfg,str(val) )
            print pred_set
            if str(sorted_memops[a]) not in pred_set:
                print "adding edge " + str(sorted_memops[a]) + "-> " + str(val)
                dfg.add_edge(sorted_memops[a],val, color='orange')
            else:
                print str(sorted_memops[a]) + " is  a pred of " + str(val)





def add_ld_edgs(a,b, sorted_memops,dfg):
    for val in sorted_memops[a:b]:
        # print "adding edges :  val_a= " + str(sorted_memops[a]) + " val_b= " + str(val)
        k = {}
        pred_set = nx.ancestors(dfg,str(sorted_memops[b]) )
        print pred_set
        if str(val) not in pred_set:
            print "adding first store edge " + str(val) + "-> " + str(sorted_memops[b])
            dfg.add_edge(val,sorted_memops[b], color='orange')
        else:
            print str(val) + " is  a pred of " + str(sorted_memops[b])






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
    b=0
    first_store=False

    for idx,val in enumerate(sorted_memops):
        if dfg.node[str(val)]['opcode']=='Store':

            b=idx
            print "a= " + str(a) + " b= " + str(b) 

            if first_store == False:
                print "first_store"
                first_store= True
                add_ld_edgs(a,b,sorted_memops,dfg)

            else:
                # print "a= " + str(a) +" val_a: "+ str(sorted_memops[a]) + " b= " + str(b) + " val_b: " + str(sorted_memops[b])
                add_edges(a,b,sorted_memops,dfg)

            a=b

    if b!=(len(sorted_memops)-1) and b!=0:
        print "last_store"
        add_edges(b,len(sorted_memops)-1,sorted_memops,dfg)




    write_dfg(dfgfile,dfg)



if __name__ == "__main__":
    main(sys.argv[1])
