#!/usr/bin/python
import sys


def main(eppfile, sbfile):
    paths = set()
    with open(eppfile,'r') as eppf:
        for l in eppf:
            s = ' '.join(l.strip().split(' ')[4:])
            paths.add(s)

    count = 0
    with open(sbfile,'r') as sbf:
        for l in sbf:
            if len(l.strip().split(' ')[4:]) > 1:
                s = ' '.join(l.strip().split(' ')[4:])
                if s not in paths:
                    print s
                    count += 1
    print count



if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
