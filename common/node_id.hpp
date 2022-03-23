#ifndef _COMMON_NODEID_H_
#define _COMMON_NODEID_H_

#include <stdint.h>

struct NodeID {
    uint8_t server_id;
    uint16_t client_id;
    uint64_t seq_nr;

    NodeID() {
        NodeID(0, 0, 0);
    }

    NodeID(uint8_t server_id, uint16_t client_id, uint64_t seq_nr) {
        this->server_id = server_id;
        this->client_id = client_id;
        this->seq_nr = seq_nr;
    }
 
    bool operator==(const NodeID &p) const {
        return server_id == p.server_id
            && client_id == p.client_id
            && seq_nr == p.seq_nr;
    }

};

struct node_id_hash_fn
{
    std::size_t operator() (const NodeID &p) const {
        std::size_t h1 = std::hash<uint8_t>()(p.server_id);
        std::size_t h2 = std::hash<uint16_t>()(p.client_id);
        std::size_t h3 = std::hash<uint64_t>()(p.seq_nr);
 
        return h1 ^ h2 ^ h3;
    }
};

#endif  /* _COMMON_NODEID_H_ */
