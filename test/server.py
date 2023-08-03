# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
from pymargo.core import Engine
from mpi4py import MPI
import pyssg

with Engine('na+sm') as engine:
    pyssg.init()
    group = pyssg.SSGGroup("mygroup1", comm=MPI.COMM_WORLD, membership_update=None)
    del group
    pyssg.finalize()
