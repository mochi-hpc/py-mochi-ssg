# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
from pymargo import MargoInstance
from mpi4py import MPI
import pyssg

mid = MargoInstance('tcp')
pyssg.init(mid)
group = pyssg.SSGGroup("mygroup1", comm=MPI.COMM_WORLD, membership_update=None)
del group
pyssg.finalize()
