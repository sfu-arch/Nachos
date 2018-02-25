#!/usr/bin/python

import sys, pprint, operator

def main(filename):
    chops = {}
    totalCov = 0
    with open(filename, 'r') as f:
        for l in f:
            s = l.strip().split(' ')
            cid = s[4] + '|' + s[-1]
            if cid not in chops.keys():
                chops[cid] = { 'ids' : [], 'fqs' : [], 'ops' : [], 'cov' : [], 'bbs' : [] }  
            chops[cid]['ids'].append(s[0])
            chops[cid]['fqs'].append(int(s[1]))
            chops[cid]['ops'].append(int(s[3]))
            chops[cid]['cov'].append(int(s[1])*int(s[3]))
            chops[cid]['bbs'].append(s[4:])
            totalCov += int(s[3]) * int(s[1])

    choplist = []
    for _,v in chops.iteritems():
        choplist.append(v) 

    choplist = sorted(choplist, key = lambda x : sum(x['cov']), reverse = True)
    
    # pp = pprint.PrettyPrinter(indent=2)
    # pp.pprint(choplist[:5])

    # id : Id of the first path in the chop
    # len : Number of paths in the chop
    # size1 : average size of paths in the chop (ops)
    # size2 : average size of paths in the chop (blocks)
    # wt : Coverage fraction of the chop

    print 'id','len','size','wt'
    for c in choplist[:5]:
        print c['ids'][0], len(c['ids']), \
            float(sum(c['ops']))/float(len(c['ids'])), \
            float(sum([len(z) for z in c['bbs']]))/float(len(c['ids'])), \
            "{0:.2f}".format(float(sum(c['cov']))/float(totalCov)*100)

    for n in range(5):
        with open('chop-seq-'+str(n)+'.txt','w') as f:
            c = choplist[n]
            for x in range(len(c['ids'])):
                f.write(' '.join([c['ids'][x], str(c['fqs'][x]), '3', str(c['ops'][x])] + c['bbs'][x] + ['\n']))
            


if __name__ == "__main__":
    main(sys.argv[1])
