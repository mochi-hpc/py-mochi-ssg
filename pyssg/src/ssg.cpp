/*
 * (C) 2018 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#define BOOST_NO_AUTO_PTR
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <margo.h>
#include <ssg.h>

BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(margo_instance)

namespace bpl = boost::python;

BOOST_PYTHON_MODULE(_pyssg)
{
    bpl::def("init", &ssg_init);
    bpl::def("finalize", &ssg_finalize);
}
