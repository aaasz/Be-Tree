#include <cstring>
#include "common/transaction.hpp"


Transaction::Transaction() :
    readSet(), writeSet(), write_values_size(0) { }


Transaction::~Transaction() { }

const ReadSetMap& Transaction::getReadSet() const {
    return readSet;
}

const WriteSetMap& Transaction::getWriteSet() const {
    return writeSet;
}

void Transaction::addToReadSet(const NodeID &node_id,
                        const Timestamp &readTime) {
    readSet[node_id] = readTime;
}

void Transaction::addToWriteSet(const NodeID &node_id,
                         const std::string &serialized_node) {
    writeSet[node_id] = serialized_node;
    write_values_size += serialized_node.size();
}

uint32_t Transaction::serialized_size() const {
    return 2*sizeof(uint16_t) + readSet.size()*sizeof(read_t) + writeSet.size()*sizeof(write_t) + write_values_size;
}

void Transaction::serialize(char *reqBuf) const {

    uint16_t *ct = (uint16_t *) reqBuf;
    *ct = readSet.size();
    ct++;
    *ct = writeSet.size();
    ct++;

    auto *read_ptr = reinterpret_cast<read_t *> (ct);
    for (auto read : readSet) {
        read_ptr->node_id = read.first;
        read_ptr->timestamp = read.second;
        read_ptr++;
    }

    auto *write_ptr = reinterpret_cast<write_t *> (read_ptr);
    for (auto write : writeSet) {
        write_ptr->node_id = write.first;
        write_ptr->size = write.second.size(); 
        std::memcpy(write_ptr->buffer, write.second.c_str(), write.second.size());
        write_ptr = (write_t*) ((char*) write_ptr + sizeof(write_ptr) + write_ptr->size);
    }
}

void Transaction::deserialize(char* buf) {

    uint16_t *ct = (uint16_t *) buf;
    uint16_t nr_reads = *ct;
    ct++;
    uint16_t nr_writes = *ct;
    ct++;

    auto *read_ptr = reinterpret_cast<read_t *> (ct);
    for (int i = 0; i < nr_reads; i++) {
        readSet[read_ptr->node_id] = read_ptr->timestamp;
        read_ptr++;
    }

    auto *write_ptr = reinterpret_cast<write_t *> (read_ptr);
    for (int i = 0; i < nr_writes; i++) {
        writeSet[write_ptr->node_id] = std::string(write_ptr->buffer, write_ptr->size);
        write_values_size += write_ptr->size;
        write_ptr = (write_t*)((char*) write_ptr + sizeof(write_ptr) + write_ptr->size);
    }
}

void Transaction::clear() {
    readSet.clear();
    writeSet.clear();
}
