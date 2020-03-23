import sys
sys.path.append('build/lib.linux-x86_64-3.7')
from pymargo.core import Engine
import pymargo.core
import pyssg
from pyssg.groups import Config, Member, Group
from callback import MyMembershipCallback

with Engine('ofi+tcp', pymargo.core.server) as engine:
    g = pyssg.groups.create('mygroup', engine, config=Config(), 
                        addresses=[ str(engine.addr()) ],
                        callback=MyMembershipCallback())
    g.dump()
    g.store('mygroup.ssg')
    engine.wait_for_finalize()
