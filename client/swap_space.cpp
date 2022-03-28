#include "swap_space.hpp"

void serialize(std::iostream &fs, serialization_context &context, uint64_t x)
{
  fs << x << " ";
  assert(fs.good());
}

void deserialize(std::iostream &fs, serialization_context &context, uint64_t &x)
{
  fs >> x;
  assert(fs.good());
}

void serialize(std::iostream &fs, serialization_context &context, int64_t x)
{
  fs << x << " ";
  assert(fs.good());
}

void deserialize(std::iostream &fs, serialization_context &context, int64_t &x)
{
  fs >> x;
  assert(fs.good());
}

void serialize(std::iostream &fs, serialization_context &context, std::string x)
{
  fs << x.size() << ",";
  assert(fs.good());
  fs.write(x.data(), x.size());
  assert(fs.good());
}

void deserialize(std::iostream &fs, serialization_context &context, std::string &x)
{
  size_t length;
  char comma;
  fs >> length >> comma;
  assert(fs.good());
  char *buf = new char[length];
  assert(buf);
  fs.read(buf, length);
  assert(fs.good());
  x = std::string(buf, length);
  delete buf;
}

bool swap_space::cmp_by_last_access(swap_space::object *a, swap_space::object *b) {
  return a->last_access < b->last_access;
}

swap_space::swap_space(backing_store *bs, StorageClient *sc, uint64_t n) :
  backstore(bs),
  sc(sc),
  max_in_memory_objects(n),
  objects(),
  lru_pqueue(cmp_by_last_access)
{}

swap_space::object::object(swap_space *sspace, serializable * tgt) {
  target = tgt;
  id = sspace->next_id++;
  bsid = 0;
  is_leaf = false;
  refcount = 1;
  last_access = sspace->next_access_time++;
  target_is_dirty = true;
  pincount = 0;
}

void swap_space::set_cache_size(uint64_t sz) {
  assert(sz > 0);
  max_in_memory_objects = sz;
  maybe_evict_something();
}

void swap_space::write_back(swap_space::object *obj)
{
  assert(objects.count(obj->id) > 0);

  Debug("Writing back %lu (%p) with last access time %lu",
        obj->id,
        obj->target,
        obj->last_access);

  // This calls _serialize on all the pointers in this object,
  // which keeps refcounts right later on when we delete them all.
  // In the future, we may also use this to implement in-memory
  // evictions, i.e. where we first "evict" an object by
  // compressing it and keeping the compressed version in memory.
  serialization_context ctxt(*this);
  std::stringstream sstream;
  serialize(sstream, ctxt, *obj->target);
  obj->is_leaf = ctxt.is_leaf;
  if (obj->target_is_dirty) {
    std::string buffer = sstream.str();
    uint64_t bsid = backstore->allocate(buffer.length());
    std::iostream *out = backstore->get(bsid);
    out->write(buffer.data(), buffer.length());
    backstore->put(out);

    sc->UpsertNode(0, obj->node_id, buffer);
    if (obj->bsid > 0)
      backstore->deallocate(obj->bsid);
    obj->bsid = bsid;
    obj->target_is_dirty = false;
  }
}

void swap_space::maybe_evict_something(void)
{
  while (current_in_memory_objects > max_in_memory_objects) {
    object *obj = NULL;
    for (auto it = lru_pqueue.begin(); it != lru_pqueue.end(); ++it)
      if ((*it)->pincount == 0) {
	obj = *it;
	break;
      }
    if (obj == NULL)
      return;
    lru_pqueue.erase(obj);

    write_back(obj);
    
    delete obj->target;
    obj->target = NULL;
    current_in_memory_objects--;
  }
}

void swap_space::evict_all(void)
{
  while (current_in_memory_objects > 0) {
    object *obj = NULL;
    for (auto it = lru_pqueue.begin(); it != lru_pqueue.end(); ++it)
      if ((*it)->pincount == 0) {
        obj = *it;
        break;
      }
    if (obj == NULL)
      return;
    lru_pqueue.erase(obj);

    write_back(obj);

    delete obj->target;
    obj->target = NULL;
    current_in_memory_objects--;
  }
}

// transactional interface implementation
void swap_space::BeginTxn() {
  Debug("Begin TXN");
  txn_started = true;
}

bool swap_space::CommitTxn() {
  bool success = true;
  uint8_t core_idx = 0;

  if (!txn_started) return false;

  Debug("Commit TXN; readSet count = %ld, writeSet count = %ld", txn.getReadSet().size(), txn.getWriteSet().size());

  // 1. lock write set
  if (sc->Lock(core_idx, txn)) {
    Debug("Locks acquired successfully");
  } else {
    success = false;
  }

  // 2. validate read set
  if (success && sc->Validate(core_idx, txn)) {
    Debug("Reads validated successfully");
  } else {
    success = false;
  }

  // 3. commit or abort
  //if (success) {
  //  sc->Commit(core_idx, txn);
  //} else {
  //  sc->Abort(core_idx, txn);
  //}

  txn.clear();
  txn_started = false;

  // TODO: support for distributed transactions

  return success;
}

void swap_space::AbortTxn() {
  if (!txn_started) return;
}
