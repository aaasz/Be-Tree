#ifndef _NETWORK_MESSAGES_H_
#define _NETWORK_MESSAGES_H_

#include "common/node_id.hpp"

// Request types
const uint8_t getNodeIdReqType = 1;
const uint8_t upsertNodeReqType = 2;
const uint8_t evictNodeReqType = 3;

struct nodeid_request_t {
    uint64_t req_nr;
};

struct nodeid_response_t {
    uint64_t req_nr;
    uint64_t id;
};

struct upsertnode_request_t {
    uint64_t req_nr;
    NodeID node_id;
    uint16_t size;
    char buffer[];
};

struct upsertnode_response_t {
    uint64_t req_nr;
    bool success;
};

struct evictnode_request_t {
    uint64_t req_nr;
    uint64_t node_id;
    char buffer[4096];
};

struct evictnode_response_t {
    uint64_t req_nr;
    bool success;
};


#endif  /* _NETWORK_MESSAGES_H_ */
