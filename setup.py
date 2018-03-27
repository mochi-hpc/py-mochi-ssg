from distutils.core import setup
from distutils.extension import Extension
from distutils.sysconfig import get_config_vars
import pkgconfig
import os
import os.path
import sys
import mpi4py

(opt,) = get_config_vars('OPT')
os.environ['OPT'] = " ".join(
		    flag for flag in opt.split() if flag != '-Wstrict-prototypes'
		)

mpi4py_path = os.path.dirname(mpi4py.__file__)

server_libs = ['boost_python']
server_libs += pkgconfig.parse('ssg')['libraries']
pymobject_server_module = Extension('_pyssg', ["pyssg/src/ssg.cpp"],
		           libraries=server_libs,
			   include_dirs=['.', mpi4py_path+'/include'],
			   depends=[])

setup(name='pyssg',
      version='0.1',
      author='Matthieu Dorier',
      description="""Python binding for SSG""",      
      ext_modules=[ pymobject_server_module ],
      packages=['pyssg']
     )
