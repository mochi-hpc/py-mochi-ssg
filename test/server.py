# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
from pymargo import MargoInstance
import pyssg

mid = MargoInstance('tcp')
pyssg.init(mid)
pyssg.finalize()
