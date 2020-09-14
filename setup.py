from distutils.core import setup
from distutils.extension import Extension
from distutils.sysconfig import get_config_vars
import pybind11
from pybind11 import get_include
import pkgconfig
import os
import os.path
import sys

def get_pybind11_include():
    path = os.path.dirname(pybind11.__file__)
    return '/'.join(path.split('/')[0:-4] + ['include'])

# Find out if MPI4PY is present
try:
    import mpi4py
    has_mpi4py = 1
    mpi4py_path = os.path.dirname(mpi4py.__file__)
    os.environ["CC"] = "mpicc"
    os.environ["CXX"] = "mpicxx"
except ImportError:
    has_mpi4py = 0

(opt,) = get_config_vars('OPT')
os.environ['OPT'] = " ".join(
		    flag for flag in opt.split() if flag != '-Wstrict-prototypes'
		)

pk = pkgconfig.parse('ssg')
libraries = pk['libraries']
library_dirs = pk['library_dirs']
include_dirs = pk['include_dirs']
include_dirs.append(".")
include_dirs.append(get_pybind11_include())

# use pybind11 built-in machinery to find header
# include paths in Python virtualenvs
# see: https://github.com/pybind/pybind11/pull/1190
include_dirs.append(get_include())

if(has_mpi4py == 1):
    include_dirs.append(mpi4py_path+'/include')

pymobject_server_module = Extension('_pyssg', ["pyssg/src/ssg.cpp"],
		           libraries=libraries,
                   library_dirs=library_dirs,
                   include_dirs=include_dirs,
                   extra_compile_args=['-std=c++11'],
                   depends=["pyssg/src/ssg.cpp"],
                   define_macros=[('HAS_MPI4PY', has_mpi4py)])

setup(name='pyssg',
      version='0.1.1',
      author='Matthieu Dorier',
      description="""Python binding for SSG""",      
      ext_modules=[ pymobject_server_module ],
      packages=['pyssg']
     )
