import sys
from pymargo.core import Engine
import pymargo.core
import pyssg
from pyssg.groups import Config, Member, Group
from callback import MyMembershipCallback

import pytest
import hypothesis
from hypothesis import given
from hypothesis import strategies as st

# use hypothesis to generate a wide range
# of string values for testing
@given(st.text())
def test_tcp_ssg_engine_names(group_name):
    # stress test the SSG group creation API
    # on values of the group name string
    # (hypothesis will try an empty string,
    # strange unicode strings, etc.)

    # NOTE: at the moment the creation of multiple
    # engine contexts from the same Python process
    # does not appear to be tolerated?
    
    # We'll need to
    # make sure each test case/example is launched from
    # its own (sub)process somehow, but so far pytest-subtests
    # is not up to the task when combined with hypothesis case
    # generation
    with Engine('ofi+tcp', pymargo.core.server) as engine:
        g = pyssg.groups.create(group_name, engine, config=Config(),
                            addresses=[ str(engine.addr()) ],
                            callback=MyMembershipCallback())

        # check a few basic properties of this ssg group
        assert g.size == 1
        assert g.rank == 0
        assert isinstance(g._Group__group_id, int)

        g.leave()
        engine.finalize()
