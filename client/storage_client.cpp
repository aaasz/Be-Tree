// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
#include "debug/assert.hpp"
#include "debug/message.hpp"
#include "client/storage_client.hpp"
#include "common/messages.hpp"

//#include <sys/time.h>
//#include <math.h>

#include <random>
using namespace std;

StorageClient::StorageClient(const network::Configuration &config,
                   network::Transport *transport,
                   uint64_t clientid)
    : config(config),
      transport(transport) {

    this->clientid = clientid;
    // Randomly generate a client ID
    // This is surely not the fastest way to get a random 64-bit int,
    // but it should be fine for this purpose.
    while (this->clientid == 0) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        this->clientid = dis(gen);
        Debug("Client ID: %lu", this->clientid);
    }
    this->lastReqId = 0;

    transport->Register(this, -1);
}

StorageClient::~StorageClient() { }


/*** RPC invocations ***/

std::string StorageClient::GetNode(uint8_t coreIdx,
                       NodeID node_id) {
    uint64_t reqId = ++lastReqId;

    // TODO: find a way to get sending errors (the eRPC's enqueue_request
    // function does not return errors)
    // TODO: deal with timeouts?
    auto *reqBuf = reinterpret_cast<getnode_request_t *>(
      transport->GetRequestBuf(
        sizeof(getnode_request_t),
        0 // TODO: compute max size of node
      )
    );

    reqBuf->req_nr = reqId;
    reqBuf->node_id = node_id;
    blocked = true;
    transport->SendRequestToServer(this,
                                    getNodeReqType,
                                    node_id.server_id, coreIdx,
                                    sizeof(getnode_request_t));
    return this->getNodeReply;
}

bool StorageClient::UpsertNode(uint8_t coreIdx,
                               NodeID node_id,
                               const string &serialized_node_buffer) {

    uint64_t reqId = ++lastReqId;
    auto *reqBuf = reinterpret_cast<upsertnode_request_t *>(
      transport->GetRequestBuf(
        sizeof(upsertnode_request_t) + serialized_node_buffer.size(),
        sizeof(upsertnode_response_t)
      )
    );
    reqBuf->req_nr = reqId;
    reqBuf->node_id = node_id;
    reqBuf->size = serialized_node_buffer.size();
    memcpy(reqBuf->buffer, serialized_node_buffer.c_str(), serialized_node_buffer.size());
    blocked = true;
    transport->SendRequestToServer(this,
                                   upsertNodeReqType,
                                   node_id.server_id, coreIdx,
                                   sizeof(upsertnode_request_t) + serialized_node_buffer.size());
    return this->upsertNodeReply;
}

bool StorageClient::Lock(uint8_t coreIdx,
                         const Transaction &txn) {

    uint64_t reqId = ++lastReqId;
    auto *reqBuf = reinterpret_cast<lock_request_t *>(
      transport->GetRequestBuf(
        sizeof(lock_request_t) + txn.serialized_size(),
        sizeof(lock_response_t)
      )
    );
    reqBuf->req_nr = reqId;
    reqBuf->size = txn.serialized_size();
    txn.serialize(reqBuf->buffer);
    blocked = true;
    transport->SendRequestToServer(this,
                                   lockReqType,
                                   0, coreIdx, // TODO: distributed transaction
                                   sizeof(lock_request_t) + txn.serialized_size());
    return this->lockReply;
}




/*** Handling RPC replies ***/

void StorageClient::ReceiveResponse(uint8_t reqType, char *respBuf) {
    Debug("[%lu] received response", clientid);
    switch(reqType){
        case getNodeReqType:
            HandleGetNodeReply(respBuf);
            break;
        case upsertNodeReqType:
            HandleUpsertNodeReply(respBuf);
            break;
        default:
            Warning("Unrecognized request type: %d\n", reqType);
    }
}

void StorageClient::HandleGetNodeReply(char *respBuf) {
    auto *resp = reinterpret_cast<getnode_response_t *>(respBuf);

    Debug(
        "Client received NodeGetNodeReplyMessage for "
        "request %lu.", resp->req_nr);

    if (resp->req_nr != lastReqId) {
        Warning(
            "Client was not expecting a GetNodeReplyMessage for request %lu, "
            "so it is ignoring the request.",
            resp->req_nr);
        return;
    }

    this->getNodeReply = std::string(resp->buffer, resp->size);
    blocked = false;
}

void StorageClient::HandleUpsertNodeReply(char *respBuf) {
    auto *resp = reinterpret_cast<upsertnode_response_t *>(respBuf);

    Debug(
        "Client received UpsertNodeReplyMessage for "
        "request %lu.", resp->req_nr);

    if (resp->req_nr != lastReqId) {
        Warning(
            "Client was not expecting an UpsertNodeReplyMessage for request %lu, "
            "so it is ignoring the request.",
            resp->req_nr);
        return;
    }
    this->upsertNodeReply = resp->success;
    blocked = false;
}

void StorageClient::HandleLockReply(char *respBuf) {
    auto *resp = reinterpret_cast<lock_response_t *>(respBuf);

    Debug(
        "Client received LockReplyMessage for "
        "request %lu.", resp->req_nr);

    if (resp->req_nr != lastReqId) {
        Warning(
            "Client was not expecting a LockReplyMessage for request %lu, "
            "so it is ignoring the request.",
            resp->req_nr);
        return;
    }
    this->lockReply = resp->success;
    blocked = false;
}
