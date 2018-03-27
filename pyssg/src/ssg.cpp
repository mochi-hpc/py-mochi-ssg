/*
 * (C) 2018 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#define BOOST_NO_AUTO_PTR
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/return_opaque_pointer.hpp>
#include <boost/python/return_value_policy.hpp>
#include <mpi.h>
#include <mpi4py/mpi4py.h>
#include <string>
#include <iostream>
#include <vector>
#include <margo.h>
#include <ssg.h>
#include <ssg-mpi.h>

namespace bpl = boost::python;

BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(margo_instance)
BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(ssg_group_descriptor)

static void pyssg_membership_update_cb(ssg_membership_update_t update, void* arg) {
    bpl::object* callable = static_cast<bpl::object*>(arg);
    if(callable->ptr() != Py_None) {
        (*callable)(update.member, (ssg_membership_update_type)update.type);
    }
}

static ssg_group_id_t pyssg_create_group(
            const std::string& group_name,
            const bpl::list& group_addr_lst,
            const bpl::object& mem_update = bpl::object()) 
{
    std::vector<std::string> addresses;
    for(unsigned i=0; i < bpl::len(group_addr_lst); i++) {
        addresses.push_back(bpl::extract<std::string>(group_addr_lst[i]));
    }
    std::vector<const char*> group_addr_strs;
    for(auto& s : addresses) {
        group_addr_strs.push_back(s.c_str());
    }
    ssg_group_id_t gid = ssg_group_create(group_name.c_str(),
            group_addr_strs.data(),
            group_addr_strs.size(),
            &pyssg_membership_update_cb,
            static_cast<void*>(new bpl::object(mem_update)));
    // XXX we are leaking memory here since there is no way to
    // delete the new-ed bpl::object when the group is destroyed
    return gid;
}

static ssg_group_id_t pyssg_create_group_from_config(
        const std::string& group_name,
        const std::string& config,
        const bpl::object& mem_update = bpl::object())
{
    ssg_group_id_t gid = ssg_group_create_config(group_name.c_str(),
            config.c_str(),
            &pyssg_membership_update_cb,
            static_cast<void*>(new bpl::object(mem_update)));
    return gid;
}

static ssg_group_id_t pyssg_create_group_from_mpi(
        const std::string& group_name,
        const bpl::object& comm,
        const bpl::object& mem_update = bpl::object())
{
    PyObject* py_comm = comm.ptr();
    MPI_Comm *comm_p = PyMPIComm_Get(py_comm);
    if (comm_p == NULL) bpl::throw_error_already_set();
    ssg_group_id_t gid = ssg_group_create_mpi(group_name.c_str(),
            *comm_p,
            &pyssg_membership_update_cb,
            static_cast<void*>(new bpl::object(mem_update)));
    if(gid == SSG_GROUP_ID_NULL) {
        std::cerr << "ERROR: could not create SSG group from MPI communicator" << std::endl;
    }
    return gid;
}

BOOST_PYTHON_MODULE(_pyssg)
{
    if (import_mpi4py() < 0) {
        std::cerr << "ERROR: could not import mpi4py from Boost Python module _pyssg" << std::endl;
        return;
    }

#define ret_policy_opaque bpl::return_value_policy<bpl::return_opaque_pointer>()

    bpl::opaque<ssg_group_descriptor>();
    bpl::enum_<ssg_membership_update_type>("update_type")
            .value("ADD", SSG_MEMBER_ADD)
            .value("REMOVE", SSG_MEMBER_REMOVE)
            ;
    bpl::def("init", &ssg_init);
    bpl::def("finalize", &ssg_finalize);
    bpl::def("create_group", &pyssg_create_group, ret_policy_opaque);
    bpl::def("create_from_mpi", &pyssg_create_group_from_mpi, ret_policy_opaque);
    bpl::def("create_from_config", &pyssg_create_group_from_config, ret_policy_opaque);
    bpl::def("free_group", &ssg_group_id_free);

#undef ret_policy_opaque
}
