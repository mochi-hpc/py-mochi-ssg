# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pyssg
import pymargo

def init(mid):
	_pyssg.init(mid._mid)

def finalize():
	_pyssg.finalize()

class SSGGroup():

	def __init__(self, name, **kwargs):
		cb = None
		if 'membership_update' in kwargs:
			cb = kwargs['membership_update']
		if 'comm' in kwargs:
			self._gid = _pyssg.create_from_mpi(name, kwargs['comm'], cb)
		elif 'config' in kwargs:
			self._gid = _pyssg.create_from_config(name, kwargs['config'], cb)
		elif 'addresses' in kwargs:
			self._gid = _pyssg.create_group(name, kwargs['addresses'], cb)
		else:
			raise RuntimeError('"comm", "config", or "addresses" should be provider')

	def __del__(self):
		_pyssg.free_group(self._gid)
