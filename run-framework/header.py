import json
import sys
import os
from collections import OrderedDict


def main(filename):
    header=False
    with open(filename, 'r') as f:
        try:
            j = json.loads(f.read(), object_pairs_hook=OrderedDict)
            name = os.path.normpath(filename).split(os.path.sep)[0]
            data = ','.join(str(v) for k,v in j.iteritems() if isinstance(v, (int, long, float)))
            if not header:
                print 'name,function,'+','.join(k for k,v in j.iteritems() if isinstance(v, (int, long, float)))
                header = True
        except ValueError as e:
            print '<'+filename+'> : '+ str(e)


if __name__ == "__main__":
    main(sys.argv[1])
