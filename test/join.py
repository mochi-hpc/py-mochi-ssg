import sys
sys.path.append('build/lib.linux-x86_64-3.7')
from pymargo.core import Engine
import pymargo.core
import pyssg
from pyssg.groups import Config, Member, Group
from callback import MyMembershipCallback

with Engine('ofi+tcp', pymargo.core.server) as engine:
    g = pyssg.groups.load('mygroup.ssg', engine)
    if len(sys.argv) > 1:
        address = sys.argv[1]
    else:
        address = None
    g.join(callback=MyMembershipCallback(), target_addr=address)
    g.dump()
    engine.wait_for_finalize()
