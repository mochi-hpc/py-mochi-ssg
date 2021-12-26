from pymargo.core import Address
import _pyssg

class Config():

    def __init__(self, **kwargs):
        self.swim_period_length_ms = kwargs.get('swim_period_length_ms', 0)
        self.swim_suspect_timeout_periods = kwargs.get('swim_suspect_timeout_periods', -1)
        self.swim_subgroup_member_count = kwargs.get('swim_subgroup_member_count', -1)
        self.ssg_credential = kwargs.get('ssg_credential', -1)

class Callback():

    def __init__(self):
        pass

    def on_join(self, member):
        pass

    def on_left(self, member):
        pass

    def on_died(self, member):
        pass

    def __call__(self, member_id, update):
        member = Member(self.group, member_id)
        if update == _pyssg.update_type.JOIN:
            self.on_join(member)
        elif update == _pyssg.update_type.LEFT:
            self.on_left(member)
        elif update == _pyssg.update_type.DIED:
            self.on_died(member)


class Member():

    def __init__(self, group, member_id):
        self.__group = group
        self.__member_id = member_id

    @property
    def member_id(self):
        return self.__member_id

    @property
    def address(self):
        a = _pyssg.group_get_member_addr(self.__group.__group_id, self.__member_id)
        return pymargo.core.Address(self.__group.__mid, a, False).copy()

    @property
    def rank(self):
        return _pyssg.group_get_member_rank(self.__group.__group_id, self.__member_id)

    @property
    def group(self):
        return self.__group

    def __str__(self):
        return _pyssg.group_get_addr_str(self.__group.__group_id, self.__member_id)


class Group():

    def __init__(self, engine, group_id, is_member=True):
        self.__mid = engine.get_internal_mid()
        self.__group_id = group_id
        self.__is_member = is_member

    def  __del__(self):
        if self.__group_id is not None:
            if self.__is_member:
                _pyssg.group_leave(self.__group_id)
            else:
                _pyssg.group_destroy(self.__group_id)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.__group_id is not None:
            if self.__is_member:
                _pyssg.group_leave(self.__group_id)
            else:
                _pyssg.group_destroy(self.__group_id)

    @property
    def is_member(self):
        return self.__is_member

    def join(self, callback, target_addr=None):
        if self.__is_member:
            raise RuntimeError('Caller is already a member')
        if target_addr is None:
            ret = _pyssg.group_join(self.__mid, self.__group_id, callback)
        else:
            ret = _pyssg.group_join_target(self.__mid, self.__group_id, str(target_addr), callback)
        if ret != 0:
            raise RuntimeError('Could not join group')
        self.__is_member = True

    def leave(self, target_addr=None):
        if not self.__is_member:
            raise RuntimeError('Caller is not a member')
        if target_addr is None:
            ret = _pyssg.group_leave(self.__group_id)
        else:
            ret = _pyssg.group_leave_target(self.__group_id, str(target_addr))
        if ret != 0:
            raise RuntimeError('Could not leave group')
        self.__is_member = False
        self.__group_id = None

    def refresh(self):
        if self.__is_member:
            pass
        _pyssg.group_refresh(self.__mid, self.__group_id)
        if ret != 0:
            raise RuntimeError('Could not observe group')

    @property
    def size(self):
        return _pyssg.group_get_size(self.__group_id)

    @property
    def rank(self):
        if not self.__is_member:
            raise RuntimeError('Caller is not a group member')
        return _pyssg.group_get_self_rank(self.__group_id)

    def __getitem__(self, key):
        if isinstance(key, slice):
            member_ids = _pyssg.group_get_member_ids_from_range(self.__group_id, key.start, key.stop)
            return [ Member(member_id) for member_id in member_ids[::key.step] ]
        else:
            member_id = _pyssg.group_get_member_id_from_rank(self.__group_id, key)
            return Member(self, member_id)

    @property
    def credentials(self):
        return _pyssg.group_id_get_cred(self.__group_id)

    def serialize(self, num_addrs=1):
        return _pyssg.group_id_serialize(self.__group_id, num_addrs)

    def store(self, filename, num_addrs=1):
        ret = _pyssg.group_id_store(filename, self.__group_id, num_addrs)
        if ret != 0:
            raise RuntimeError('Could not store group in '+str(filename))

    def dump(self):
        _pyssg.group_dump(self.__group_id)

def __create_simple(engine, group_name, config, addresses, callback):
    ssgid = _pyssg.group_create(engine.get_internal_mid(), group_name, addresses, vars(config), callback)
    return Group(engine, ssgid)

def __create_file(engine, group_name, config, filename, callback):
    ssgid = _pyssg.group_create_config(engine.get_internal_mid(), group_name, filename, vars(config), callback)
    return Group(engine, ssgid)

def __create_mpi(engine, group_name, config, comm, callback):
    ssgid = _pyssg.group_create_mpi(engine.get_internal_mid(), group_name, comm, vars(config), callback)
    return Group(engine, ssgid)

def create(group_name, engine, config=Config(), callback=None, **kwargs):
    if 'addresses' in kwargs:
        g = __create_simple(engine, group_name, config, kwargs['addresses'], callback)
    elif 'filename' in kwargs:
        g = __create_file(engine, group_name, config, kwargs['filename'], callback)
    elif 'communicator' in kwargs:
        g = __create_mpi(engine, group_name, config, kwargs['communicator'], callback)
    else:
        raise ValueError('Either "addresses", "filename", or "communicator" must be provided')
    if isinstance(callback, Callback):
        setattr(callback, 'group', g)
    return g

def deserialize(group_str, engine, num_addrs=1):
    gid = _pyssg.group_id_deserialize(group_str, num_addrs)
    if gid == 0:
        raise RuntimeError('Could not deserialize group')
    return Group(engine, gid, False)

def load(filename, engine, num_addrs=1):
    gid = _pyssg.group_id_load(filename, num_addrs)
    if gid == 0:
        raise RuntimeError('Could not load group')
    return Group(engine, gid, False)

