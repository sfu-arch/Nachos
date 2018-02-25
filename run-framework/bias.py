#!/usr/bin/python

import sys, pprint, operator

def main(filename):
    pp = pprint.PrettyPrinter(indent=2)
    edges = {}
    with open(filename, 'r') as f:
        for l in f:
            s = l.strip().split(' ')
            for x in range(4, len(s) - 1):
                k = (s[x],s[x+1])
                if k not in edges.keys():
                    edges[k] = 0
                edges[k] += int(s[1])
    
    
    # All gather all the blocks terminated by a 
    # conditional branch and find their bias.
    br = {}
    for k,v in edges.iteritems():
        b1, b2 = k 
        if b1 not in br.keys():
            br[b1] = {}
        if b2 not in br[b1].keys():
            br[b1][b2] = 0
        br[b1][b2] = v

    # pp.pprint(br)
    all_total = 0
    for _,v in br.iteritems():
        if len(v.keys()) == 2:
            a = v.keys()[0]
            b = v.keys()[1]
            all_total += float(v[a] + v[b])
   
    cbr = {} 
    for k,v in br.iteritems():
        if len(v.keys()) == 2:
            a = v.keys()[0]
            b = v.keys()[1]
            tot = float(v[a] + v[b])
            if tot > 0.1 * all_total:
                cbr[k] = (v[a]/tot*100, v[b]/tot*100)  

    print 'block', 'if', 'else'
    for k,v in cbr.iteritems():
        print k, "{0:.2f}".format(v[0]), "{0:.2f}".format(v[1])


if __name__ == "__main__":
    main(sys.argv[1])
