// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 *   Store server hosting the nodes of the B3-tree
 *
 **********************************************************************/
#include "storage_server.hpp"
#include "common/messages.hpp"
#include <iostream>
#include <string.h>

using namespace std;

StorageServerApp::StorageServerApp() : current_id(0) {

}

std::string StorageServerApp::GetNode(NodeID node_id) {
    return objects[node_id].serialized_node;
}

void StorageServerApp::UpsertNode(NodeID node_id, uint16_t size, char* buff) {
    objects[node_id].serialized_node = std::string(buff, size);
}

bool StorageServerApp::Lock(Transaction &txn) {
    for (auto w : txn.getWriteSet()) {
        if (objects[w.first].locked) {
            Debug("Node already locked <%d, %d, %ld>", w.first.server_id, w.first.client_id, w.first.seq_nr);
            return false;
        }
        objects[w.first].locked = true;
        Debug("Locked <%d, %d, %ld>", w.first.server_id, w.first.client_id, w.first.seq_nr);
    }
    return true;
}

bool StorageServerApp::Validate(Transaction &txn) {
    for (auto r : txn.getReadSet()) {
        if (objects[r.first].locked) {
            Debug("Node locked <%d, %d, %ld>", r.first.server_id, r.first.client_id, r.first.seq_nr);
            return false;
        }
        if (objects[r.first].ts.version != r.second.version) {
            Debug("Versions do not match %ld, %ld", objects[r.first].ts.version, r.second.version);
            return false;
        }
    }
    return true;
}

StorageServer::StorageServer(network::Configuration config, int myIdx,
                     network::Transport *transport,
                     StorageServerApp *storageApp)
    : config(std::move(config)), myIdx(myIdx), transport(transport),
      storageApp(storageApp)
{
    if (transport != NULL) {
        transport->Register(this, myIdx);
    } else {
        // we use this for micorbenchmarking, but still issue a warning
        Warning("NULL transport provided to the replication layer");
    }
}

StorageServer::~StorageServer() { }

void StorageServer::ReceiveRequest(uint8_t reqType, char *reqBuf, char *respBuf) {
    size_t respLen;
    switch(reqType) {
        case getNodeReqType:
            HandleGetNode(reqBuf, respBuf, respLen);
            break;
        case upsertNodeReqType:
            HandleUpsertNode(reqBuf, respBuf, respLen);
            break;
        case lockReqType:
            HandleLock(reqBuf, respBuf, respLen);
            break;
        case validateReqType:
            HandleValidate(reqBuf, respBuf, respLen);
            break;
        default:
            Warning("Unrecognized rquest type: %d", reqType);
    }

    // For every request, we need to send a response (because we use eRPC)
    if (!(transport->SendResponse(respLen)))
        Warning("Failed to send reply message");
}

void StorageServer::HandleGetNode(char *reqBuf, char *respBuf, size_t &respLen) {
    Debug("Received get node");
    auto *req = reinterpret_cast<getnode_request_t *>(reqBuf);
    auto *resp = reinterpret_cast<getnode_response_t *>(respBuf);
    resp->req_nr = req->req_nr;
    std::string node = storageApp->GetNode(req->node_id);
    resp->size = node.size();
    memcpy(resp->buffer, node.c_str(), node.size());
    respLen = sizeof(getnode_response_t) + node.size();
}

void StorageServer::HandleUpsertNode(char *reqBuf, char *respBuf, size_t &respLen) {
    Debug("Received HandleUpsertNode");
    auto *req = reinterpret_cast<upsertnode_request_t *>(reqBuf);
    auto *resp = reinterpret_cast<upsertnode_response_t *>(respBuf);
    NodeID nid = req->node_id;
    Debug("NodeId = <%d, %d, %ld>", nid.server_id, nid.client_id, nid.seq_nr);
    storageApp->UpsertNode(req->node_id, req->size, req->buffer);
    resp->req_nr = req->req_nr;
    resp->success = true;
    respLen = sizeof(upsertnode_response_t);
}

// lock all nodes in the transaction's write set
void StorageServer::HandleLock(char *reqBuf, char *respBuf, size_t &respLen) {
    Debug("Received HandleLock");
    auto *req = reinterpret_cast<lock_request_t *>(reqBuf);
    auto *resp = reinterpret_cast<lock_response_t *>(respBuf);
    Transaction t;
    t.deserialize(req->buffer);
    bool success = storageApp->Lock(t);
    resp->req_nr = req->req_nr;
    resp->success = success;
    respLen = sizeof(lock_response_t);
}

// validate reads in the transaction's read set
void StorageServer::HandleValidate(char *reqBuf, char *respBuf, size_t &respLen) {
    Debug("Received HandleValidate");
    auto *req = reinterpret_cast<validate_request_t *>(reqBuf);
    auto *resp = reinterpret_cast<validate_response_t *>(respBuf);
    Transaction t;
    t.deserialize(req->buffer);
    bool success = storageApp->Validate(t);
    resp->req_nr = req->req_nr;
    resp->success = success;
    respLen = sizeof(validate_response_t);
}
