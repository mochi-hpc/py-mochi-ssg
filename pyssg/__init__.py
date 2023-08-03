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


get_group_cred_from_file = _pyssg.get_group_cred_from_file
get_group_cred_from_buf = _pyssg.get_group_cred_from_buf
get_group_transport_from_file = _pyssg.get_group_transport_from_file
get_group_transport_from_buf = _pyssg.get_group_transport_from_buf
