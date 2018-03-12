#!/usr/bin/python
import sys


def main(dotfile):
    vertex= []
    vertex_line= []
    edges= []
    with open(dotfile) as f:
        for line in f:
            line=line.rstrip()
            if "vCDEcomp" in line:
                vertex.append(line.partition(' ')[0])
                vertex_line.append(line)

            elif "ecde_comparator_delay" in line:
                edge=(line.partition(' ')[0])
                src_vertex=(edge.partition('->')[0])
                tgt_vertex=(edge.partition('->')[2])
                edges.append([src_vertex,tgt_vertex,line])
    
    
    
    print "digraph G {"

    for line in vertex_line:
        nodeid=line.partition('[')[0]
        attr=line.partition('[')[2]
        attr0=attr.partition(',')[0]
        attr1_meta=attr.partition(',')[2]
        attr1=attr1_meta.partition(',')[0]
        # print attr0  + "    " + attr1
        print nodeid + "[ label= \""+ nodeid +"(" + attr0 + attr1 + ")\"," + attr

    for each_edge in edges:
        src_vertex=each_edge[0]
        tgt_vertex=each_edge[1]

        if src_vertex in vertex and tgt_vertex in vertex:
                # print src_vertex + " " + tgt_vertex
                print each_edge[2]

    print "}"
        



if __name__ == "__main__":
    main(sys.argv[1])
