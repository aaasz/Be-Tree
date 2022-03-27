#ifndef _COMMON_TRANSACTION_H_
#define _COMMON_TRANSACTION_H_


#include <stdint.h>
#include <unordered_map>
#include "common/node_id.hpp"

struct Timestamp{
    uint64_t version;

    Timestamp() {
        version = 0;
    } 
};

// TODO: parametrize the class
//template <class Key> using ReadSetMap = std::unordered_map<Key, Timestamp> ;
//template <class Key, class Value> using WriteSetMap = std::unordered_map<Key, Value> ;

typedef std::unordered_map<NodeID, Timestamp, node_id_hash_fn> ReadSetMap;
typedef std::unordered_map<NodeID, std::string, node_id_hash_fn> WriteSetMap;

class Transaction {
private:
    // map between key and timestamp at
    // which the read happened and how
    // many times this key has been read
    ReadSetMap readSet;

    // map between key and value(s)
    //std::unordered_map<std::string, std::string> writeSet;
    WriteSetMap writeSet;

    uint32_t write_values_size;

public:
    Transaction();
    ~Transaction();

    //const std::unordered_map<std::string, Timestamp>& getReadSet() const;
    const ReadSetMap& getReadSet() const;
    //const std::unordered_map<std::string, std::string>& getWriteSet() const;
    const WriteSetMap& getWriteSet() const;

    void addToReadSet(const NodeID &node_id, const Timestamp &readTime);
    void addToWriteSet(const NodeID &node_id, const std::string &serialized_node);
    uint32_t serialized_size() const;
    void serialize(char *buffer) const;
    void deserialize(char *buffer);
    void clear();
};

// transations are serialized to a buffer containing arrays
// of these structures:
struct read_t {
    NodeID node_id; 
    Timestamp timestamp;
};

struct write_t {
    NodeID node_id;
    uint16_t size;
    char buffer[];
};

#endif /* _COMMON_TRANSACTION_H_ */
