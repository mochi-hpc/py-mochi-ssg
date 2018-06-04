/*
 * (C) 2018 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <pybind11/pybind11.h>
#if HAS_MPI4PY
#include <mpi.h>
#include <mpi4py/mpi4py.h>
#endif
#include <string>
#include <iostream>
#include <vector>
#include <margo.h>
#include <ssg.h>
#if HAS_MPI4PY
#include <ssg-mpi.h>
#endif

namespace py11 = pybind11;

typedef py11::capsule pymargo_instance_id;
typedef py11::capsule pyssg_group_id_t;

#define MID2CAPSULE(__mid) py11::capsule((void*)(__mid), "margo_instance_id", nullptr)
#define SSGID2CAPSULE(__ssgid) py11::capsule((void*)(__ssgid), "pyssg_group_id_t", nullptr)

static void pyssg_membership_update_cb(ssg_membership_update_t update, void* arg) {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    py11::object* callable = static_cast<py11::object*>(arg);
    if(callable->ptr() != Py_None) {
        (*callable)(update.member, (ssg_membership_update_type)update.type);
    }
    PyGILState_Release(gstate);
}

static pyssg_group_id_t pyssg_create_group(
            const std::string& group_name,
            const py11::list& group_addr_lst,
            const py11::object& mem_update = py11::object()) 
{
    std::vector<std::string> addresses;
    for(unsigned i=0; i < py11::len(group_addr_lst); i++) {
        addresses.push_back(py11::cast<std::string>(group_addr_lst[i]));
    }
    std::vector<const char*> group_addr_strs;
    for(auto& s : addresses) {
        group_addr_strs.push_back(s.c_str());
    }
    
    void* uarg = static_cast<void*>(new py11::object(mem_update));
    ssg_group_id_t gid;
    Py_BEGIN_ALLOW_THREADS
    gid = ssg_group_create(group_name.c_str(),
            group_addr_strs.data(),
            group_addr_strs.size(),
            &pyssg_membership_update_cb,
            uarg);
    Py_END_ALLOW_THREADS
    // XXX we are leaking memory here since there is no way to
    // delete the new-ed py11::object when the group is destroyed
    return SSGID2CAPSULE(gid);
}

static pyssg_group_id_t pyssg_create_group_from_config(
        const std::string& group_name,
        const std::string& config,
        const py11::object& mem_update = py11::object())
{
    ssg_group_id_t gid;
    void* uarg = static_cast<void*>(new py11::object(mem_update));
    Py_BEGIN_ALLOW_THREADS
    gid = ssg_group_create_config(group_name.c_str(),
            config.c_str(),
            &pyssg_membership_update_cb,
            uarg);
    Py_END_ALLOW_THREADS
    return SSGID2CAPSULE(gid);
}

#if HAS_MPI4PY
static pyssg_group_id_t pyssg_create_group_from_mpi(
        const std::string& group_name,
        const py11::object& comm,
        const py11::object& mem_update = py11::object())
{
    PyObject* py_comm = comm.ptr();
    MPI_Comm *comm_p = PyMPIComm_Get(py_comm);
    if (comm_p == NULL) throw py11::error_already_set();
    ssg_group_id_t gid;
    void* uarg = static_cast<void*>(new py11::object(mem_update));
    Py_BEGIN_ALLOW_THREADS
    gid = ssg_group_create_mpi(group_name.c_str(),
            *comm_p,
            &pyssg_membership_update_cb,
            uarg);
    Py_END_ALLOW_THREADS
    if(gid == SSG_GROUP_ID_NULL) {
        std::cerr << "ERROR: could not create SSG group from MPI communicator" << std::endl;
    }
    return SSGID2CAPSULE(gid);
}
#endif

PYBIND11_MODULE(_pyssg, m)
{
#if HAS_MPI4PY
    if (import_mpi4py() < 0) {
        std::cerr << "ERROR: could not import mpi4py from Boost Python module _pyssg" << std::endl;
        return;
    }
#endif

    py11::enum_<ssg_membership_update_type>(m,"update_type")
            .value("ADD", SSG_MEMBER_ADD)
            .value("REMOVE", SSG_MEMBER_REMOVE)
            ;
    m.def("init", [](pymargo_instance_id mid) { return ssg_init(mid); } );
    m.def("finalize", &ssg_finalize);
    m.def("create_group", &pyssg_create_group);
#if HAS_MPI4PY
    m.def("create_from_mpi", &pyssg_create_group_from_mpi);
#endif
    m.def("create_from_config", &pyssg_create_group_from_config);
    m.def("free_group", [](pyssg_group_id_t gid) { return ssg_group_id_free(gid); });

}
