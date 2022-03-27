#ifndef _NETWORK_MESSAGES_H_
#define _NETWORK_MESSAGES_H_

#include "common/node_id.hpp"

// Request types
const uint8_t getNodeReqType = 1;
const uint8_t upsertNodeReqType = 2;
const uint8_t lockReqType = 3;

struct getnode_request_t {
    uint64_t req_nr;
    NodeID node_id;
};

struct getnode_response_t {
    uint64_t req_nr;
    uint16_t size;
    char buffer[];
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

struct lock_request_t {
    uint64_t req_nr;
    uint16_t size;
    char buffer[];
};

struct lock_response_t {
    uint64_t req_nr;
    bool success;
};

#endif  /* _NETWORK_MESSAGES_H_ */
