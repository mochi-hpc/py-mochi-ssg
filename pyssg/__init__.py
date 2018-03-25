# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pyssg
import pymargo

def init(mid):
	_pyssg.init(mid._mid)

def finalize():
	_pyssg.finalize()
