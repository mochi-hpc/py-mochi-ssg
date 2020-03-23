# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pyssg
import atexit
import os

__ssg_is_initialized = False

def init():
    global __ssg_is_initialized
    if not __ssg_is_initialized:
        _pyssg.init()
    __ssg_is_initialized = True

def finalize():
    global __ssg_is_initialized
    if __ssg_is_initialized:
        _pyssg.finalize()
    __ssg_is_initialized = False

__auto_init = int(os.environ.get('PYSSG_AUTO_INIT', 1))
if (__auto_init == 1):
    init()
    atexit.register(finalize)
