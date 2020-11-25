import pymargo.core
from pymargo.core import Engine
import pyssg
from pyssg.groups import Config
from callback import MyMembershipCallback

from hypothesis import example, given
from hypothesis import strategies as st

# use hypothesis to generate a wide range
# of string values and integers for testing
@given(
    group_name=st.text(),
    period_length=st.integers(),
    sus_timeout=st.integers(),
    member_ct=st.integers(),
    ssg_cred=st.integers(),
    )
def test_tcp_ssg_engine(
        group_name,
        period_length,
        sus_timeout,
        member_ct,
        ssg_cred,
        ):
    # stress test the SSG group creation API
    # on values of the group name string
    # (hypothesis will try an empty string,
    # strange unicode strings, etc.)

    with Engine('ofi+tcp', pymargo.core.server) as engine:

        # stress test the SSG group configuration
        gconfig = Config()
        gconfig.swim_period_length_ms = period_length       # default is 0
        gconfig.swim_suspect_timeout_periods = sus_timeout  # default is -1
        gconfig.swim_subgroup_member_count = member_ct      # default is -1
        gconfig.ssg_credential = ssg_cred                   # default is -1

        try:
            # try to create the group with hypothesis generated group names
            # and configurations
            g = pyssg.groups.create(group_name, engine, config=gconfig,
                                    addresses=[ str(engine.addr()) ],
                                    callback=MyMembershipCallback())
        except TypeError as err:
            # catch the generated TypeError
            print(err)
            # assert that the TypeError caught is correct
            assert "incompatible function arguments. The following argument types are supported:" in str(err)
        else:
            # if no error is caught, check a few basic properties of this ssg group
            assert g.size == 1
            assert g.rank == 0
            assert isinstance(g._Group__group_id, int)
            g.leave()
            engine.finalize()
