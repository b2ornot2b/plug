#        !/usr/bin/env python3

import sys
sys.path.insert(0, "/home/b2/work/mine/mustang/piorekf-plug/piorekf-plug-7de1969d38ab/build/lib.linux-x86_64-3.6")
print(sys.path)

from fendermustang import FenderMustang

def do_test():
    fm = FenderMustang()
    ret = fm.start()
    print(ret)

    import code; code.interact(local=locals())

if __name__ == '__main__':
    do_test()
