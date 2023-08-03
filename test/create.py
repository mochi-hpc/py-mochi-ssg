import sys
from pymargo.core import Engine
import pymargo.core
import pyssg
from pyssg.groups import Config, Member, Group
from callback import MyMembershipCallback

with Engine('na+sm', pymargo.core.server) as engine:
    pyssg.init()
    g = pyssg.groups.create('mygroup', engine, config=Config(),
                        addresses=[ str(engine.addr()) ],
                        callback=MyMembershipCallback())
    g.dump()
    g.store('mygroup.ssg')
    del g
    pyssg.finalize()
    engine.wait_for_finalize()
