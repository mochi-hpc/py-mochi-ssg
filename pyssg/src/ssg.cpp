/*
 * (C) 2018 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <pybind11/pybind11.h>
#include <pybind11/iostream.h>
#include <pybind11/stl.h>
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
#include <string>
#ifdef COLZA_ENABLE_DRC
extern "C" {
#include <rdmacred.h>
}
#endif

using namespace std::string_literals;
namespace py11 = pybind11;

typedef py11::capsule pymargo_instance_id;
typedef py11::capsule pymargo_addr;

#define MID2CAPSULE(__mid) py11::capsule((void*)(__mid), "margo_instance_id", nullptr)
#define ADDR2CAPSULE(__addr) py11::capsule((void*)(__addr), "hg_addr_t", nullptr)

static void pyssg_membership_update_cb(void* group_data, ssg_member_id_t member_id, ssg_member_update_type_t update_type) {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    py11::object* callable = static_cast<py11::object*>(group_data);
    if(callable->ptr() != Py_None) {
        (*callable)(member_id, update_type);
    }
    PyGILState_Release(gstate);
}

static ssg_group_id_t pyssg_group_create(
                            pymargo_instance_id mid,
                            const std::string& group_name,
                            const std::vector<std::string>& group_addr_strs,
                            std::map<std::string,int64_t>& group_conf,
                            const py11::object& update_cb = py11::object())
{

    std::vector<const char*> group_addr_strs_v(group_addr_strs.size());
    for(unsigned i=0; i < group_addr_strs.size(); i++)
        group_addr_strs_v[i] = group_addr_strs[i].c_str();

    ssg_group_config_t gconf = SSG_GROUP_CONFIG_INITIALIZER;
    if(group_conf.count("swim_period_length_ms"))
        gconf.swim_period_length_ms = group_conf["swim_period_length_ms"];
    if(group_conf.count("swim_suspect_timeout_periods"))
        gconf.swim_suspect_timeout_periods = group_conf["swim_suspect_timeout_periods"];
    if(group_conf.count("swim_subgroup_member_count"))
        gconf.swim_subgroup_member_count = group_conf["swim_subgroup_member_count"];
    if(group_conf.count("ssg_credential"))
        gconf.ssg_credential = group_conf["ssg_credential"];

    void* update_cb_dat = static_cast<void*>(new py11::object(update_cb));

    ssg_group_id_t ssgid = SSG_GROUP_ID_INVALID;
    int ret = ssg_group_create(
            mid,
            group_name.c_str(),
            group_addr_strs_v.data(),
            group_addr_strs_v.size(),
            &gconf,
            pyssg_membership_update_cb,
            update_cb_dat,
            &ssgid);
    if(ret != SSG_SUCCESS) {
        throw std::runtime_error("ssg_group_create_mpi returned "s + std::to_string(ret));
    }
    return ssgid;
}

static ssg_group_id_t pyssg_group_create_config(
                            pymargo_instance_id mid,
                            const std::string& group_name,
                            const std::string& file_name,
                            std::map<std::string,int64_t>& group_conf,
                            const py11::object& update_cb = py11::object())
{
    ssg_group_config_t gconf = SSG_GROUP_CONFIG_INITIALIZER;
    if(group_conf.count("swim_period_length_ms"))
        gconf.swim_period_length_ms = group_conf["swim_period_length_ms"];
    if(group_conf.count("swim_suspect_timeout_periods"))
        gconf.swim_suspect_timeout_periods = group_conf["swim_suspect_timeout_periods"];
    if(group_conf.count("swim_subgroup_member_count"))
        gconf.swim_subgroup_member_count = group_conf["swim_subgroup_member_count"];
    if(group_conf.count("ssg_credential"))
        gconf.ssg_credential = group_conf["ssg_credential"];

    void* update_cb_dat = static_cast<void*>(new py11::object(update_cb));

    ssg_group_id_t ssgid = SSG_GROUP_ID_INVALID;
    int ret = ssg_group_create_config(
            mid,
            group_name.c_str(),
            file_name.c_str(),
            &gconf,
            pyssg_membership_update_cb,
            update_cb_dat, &ssgid);
    if(ret != SSG_SUCCESS) {
        throw std::runtime_error("ssg_group_create_mpi returned "s + std::to_string(ret));
    }

    return ssgid;
}

#if HAS_MPI4PY
static ssg_group_id_t pyssg_group_create_mpi(
        pymargo_instance_id mid,
        const std::string& group_name,
        const py11::object& comm,
        std::map<std::string,int64_t>& group_conf,
        const py11::object& mem_update = py11::object())
{
    PyObject* py_comm = comm.ptr();
    MPI_Comm *comm_p = PyMPIComm_Get(py_comm);
    if (comm_p == NULL) throw py11::error_already_set();
    ssg_group_id_t gid;

    void* uarg = static_cast<void*>(new py11::object(mem_update));

    ssg_group_config_t gconf = SSG_GROUP_CONFIG_INITIALIZER;
    if(group_conf.count("swim_period_length_ms"))
        gconf.swim_period_length_ms = group_conf["swim_period_length_ms"];
    if(group_conf.count("swim_suspect_timeout_periods"))
        gconf.swim_suspect_timeout_periods = group_conf["swim_suspect_timeout_periods"];
    if(group_conf.count("swim_subgroup_member_count"))
        gconf.swim_subgroup_member_count = group_conf["swim_subgroup_member_count"];
    if(group_conf.count("ssg_credential"))
        gconf.ssg_credential = group_conf["ssg_credential"];

    int ret = ssg_group_create_mpi(
            mid,
            group_name.c_str(),
            *comm_p,
            &gconf,
            &pyssg_membership_update_cb,
            uarg, &gid);
    if(ret != SSG_SUCCESS) {
        throw std::runtime_error("ssg_group_create_mpi returned "s + std::to_string(ret));
    }
    return gid;
}
#endif

static int pyssg_group_destroy(ssg_group_id_t group_id) {
    return ssg_group_destroy(group_id);
}

static int pyssg_group_join(pymargo_instance_id mid,
                          ssg_group_id_t group_id,
                          const py11::object& update_cb)
{
    py11::scoped_ostream_redirect stream(
        std::cerr,
        py11::module::import("sys").attr("stderr")
    );
    void* update_cb_dat = static_cast<void*>(new py11::object(update_cb));
    return ssg_group_join(mid, group_id, pyssg_membership_update_cb, update_cb_dat);
}

static int pyssg_group_join_target(
        pymargo_instance_id mid,
        ssg_group_id_t group_id,
        const std::string& target_addr_str,
        const py11::object& update_cb)
{
    void* update_cb_dat = static_cast<void*>(new py11::object(update_cb));
    return ssg_group_join_target(mid, group_id, target_addr_str.c_str(), pyssg_membership_update_cb, update_cb_dat);
}

static int pyssg_group_leave(ssg_group_id_t group_id) {
    return ssg_group_leave(group_id);
}

static int pyssg_group_leave_target(ssg_group_id_t group_id, const std::string& addr) {
    return ssg_group_leave_target(group_id, addr.c_str());
}

static int pyssg_group_observe(pymargo_instance_id mid, ssg_group_id_t group_id) {
    return ssg_group_observe(mid, group_id);
}

static int pyssg_group_observe_target(pymargo_instance_id mid, ssg_group_id_t group_id, const std::string& target_addr_str) {
    return ssg_group_observe_target(mid, group_id, target_addr_str.c_str());
}

static int pyssg_group_unobserve(ssg_group_id_t group_id) {
    return ssg_group_unobserve(group_id);
}

static ssg_member_id_t pyssg_get_self_id(pymargo_instance_id mid) {
    ssg_member_id_t member_id;
    int ret = ssg_get_self_id(mid, &member_id);
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_self_id returned "s + std::to_string(ret));
    return member_id;
}

static int pyssg_get_group_size(ssg_group_id_t ssgid) {
    int size;
    int ret = ssg_get_group_size(ssgid, &size);
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_group_size returned "s + std::to_string(ret));
    return size;
}

static pymargo_addr pyssg_get_group_member_addr(ssg_group_id_t group_id, ssg_member_id_t member_id) {
    hg_addr_t addr = HG_ADDR_NULL;
    int ret = ssg_get_group_member_addr(group_id, member_id, &addr);
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_group_member_addr returned "s + std::to_string(ret));
    return ADDR2CAPSULE(addr);
}

static int pyssg_get_group_self_rank(ssg_group_id_t group_id) {
    int rank;
    int ret = ssg_get_group_self_rank(group_id, &rank);
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_group_self_rank returned "s + std::to_string(ret));
    return rank;
}

static int pyssg_get_group_member_rank(ssg_group_id_t group_id, ssg_member_id_t member_id) {
    int rank;
    int ret = ssg_get_group_member_rank(group_id, member_id, &rank);
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_group_member_rank returned "s + std::to_string(ret));
    return rank;
}

static ssg_member_id_t pyssg_get_group_member_id_from_rank(ssg_group_id_t group_id, int rank) {
    ssg_member_id_t member_id;
    int ret = ssg_get_group_member_id_from_rank(group_id, rank, &member_id);
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_group_member_id_from_rank returned "s + std::to_string(ret));
    return member_id;
}

static std::vector<ssg_member_id_t> pyssg_get_group_member_ids_from_range(ssg_group_id_t group_id,
        int rank_start,
        int rank_end) {
    std::vector<ssg_member_id_t> result(rank_end-rank_start);
    int ret = ssg_get_group_member_ids_from_range(group_id, rank_start, rank_end, result.data());
    if(ret != SSG_SUCCESS)
        throw std::runtime_error("ssg_get_group_member_ids_from_range returned "s + std::to_string(ret));
    return result;
}

static std::string pyssg_group_id_get_addr_str(ssg_group_id_t group_id, unsigned int addr_index) {
    char* addr = NULL;
    int ret = ssg_group_id_get_addr_str(group_id, addr_index, &addr);
    if(ret != SSG_SUCCESS) {
        throw std::runtime_error("ssg_group_id_get_addr_str returned "s + std::to_string(ret));
    }
    auto addr_str = std::string(addr);
    free(addr);
    return addr_str;
}

static int64_t pyssg_group_id_get_cred(ssg_group_id_t group_id) {
    return pyssg_group_id_get_cred(group_id);
}

static std::string pyssg_group_id_serialize(ssg_group_id_t group_id, int num_addrs) {
    char* buf = nullptr;
    size_t buf_size = 0;
    ssg_group_id_serialize(group_id, num_addrs, &buf, &buf_size);
    std::string result(buf);
    free(buf);
    return result;
}

static ssg_group_id_t pyssg_group_id_deserialize(const std::string& buf, int num_addrs) {
    ssg_group_id_t ssgid;
    ssg_group_id_deserialize(buf.data(), buf.size(), &num_addrs, &ssgid);
    return ssgid;
}

static int pyssg_group_id_store(const std::string& file_name, ssg_group_id_t group_id, int num_addrs) {
    return ssg_group_id_store(file_name.c_str(), group_id, num_addrs);
}

static ssg_group_id_t pyssg_group_id_load(const std::string& file_name, int num_addrs) {
    ssg_group_id_t ssgid;
    int ret = ssg_group_id_load(file_name.c_str(), &num_addrs, &ssgid);
    if(ret != 0)
        throw std::runtime_error(std::string("ssg_group_id_load returned ")+std::to_string(ret));
    return ssgid;
}

static void pyssg_group_dump(ssg_group_id_t group_id) {
    ssg_group_dump(group_id);
}

uint32_t pyssg_get_credentials_from_ssg_file(const std::string& filename) {
    uint32_t cookie = 0;
#ifdef COLZA_ENABLE_DRC
    int num_addrs = 1;
    ssg_group_id_t gid;
    int ret = ssg_group_id_load(filename.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS) {
        throw std::runtime_error("Could not load SSG group id from file");
    }
    int64_t credential_id = -1;
    ret = ssg_group_id_get_cred(gid, &credential_id);
    if(credential_id == -1)
        return cookie;
    //ssg_group_destroy(gid);

    drc_info_handle_t drc_credential_info;

    ret = drc_access(credential_id, 0, &drc_credential_info);
    if(ret != DRC_SUCCESS) {
        throw std::runtime_error("drc_access failed");
    }

    cookie = drc_get_first_cookie(drc_credential_info);
#endif
    return cookie;
}

PYBIND11_MODULE(_pyssg, m)
{
#if HAS_MPI4PY
    if (import_mpi4py() < 0) {
        std::cerr << "ERROR: could not import mpi4py from Boost Python module _pyssg" << std::endl;
        return;
    }
#endif

    py11::enum_<ssg_member_update_type>(m,"update_type")
            .value("JOIN", SSG_MEMBER_JOINED)
            .value("LEFT", SSG_MEMBER_LEFT)
            .value("DIED", SSG_MEMBER_DIED)
            ;
    m.def("init", &ssg_init);
    m.def("finalize", &ssg_finalize);
    m.def("get_self_id", &pyssg_get_self_id);
    m.def("group_create", &pyssg_group_create);
#if HAS_MPI4PY
    m.def("group_create_mpi", &pyssg_group_create_mpi);
#endif
    m.def("group_create_config", &pyssg_group_create_config);
    m.def("group_destroy", &pyssg_group_destroy);
    m.def("group_join", &pyssg_group_join);
    m.def("group_join_target", &pyssg_group_join_target);
    m.def("group_leave", &pyssg_group_leave);
    m.def("group_leave_target", &pyssg_group_leave_target);
    m.def("group_observe", &pyssg_group_observe);
    m.def("group_observe_target", &pyssg_group_observe_target);
    m.def("group_unobserve", &pyssg_group_unobserve);
    m.def("group_get_size", &pyssg_get_group_size);
    m.def("group_get_member_addr", &pyssg_get_group_member_addr);
    m.def("group_get_self_rank", &pyssg_get_group_self_rank);
    m.def("group_get_member_rank", &pyssg_get_group_member_rank);
    m.def("group_get_member_id_from_rank", &pyssg_get_group_member_id_from_rank);
    m.def("group_get_member_ids_from_range", &pyssg_get_group_member_ids_from_range);
    m.def("group_id_get_addr_str", &pyssg_group_id_get_addr_str);
    m.def("group_id_get_cred", &pyssg_group_id_get_cred);
    m.def("group_id_serialize", &pyssg_group_id_serialize);
    m.def("group_id_deserialize", &pyssg_group_id_deserialize);
    m.def("group_id_store", &pyssg_group_id_store);
    m.def("group_id_load", &pyssg_group_id_load);
    m.def("group_dump", &pyssg_group_dump);
    m.def("get_credentials_from_ssg_file", &pyssg_get_credentials_from_ssg_file);
}
