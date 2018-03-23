import _pyssg
import pymargo

def init(mid):
	_pyssg.init(mid._mid)

def finalize():
	_pyssg.finalize()
