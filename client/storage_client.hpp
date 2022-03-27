// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
#ifndef _STORAGE_CLIENT_H_
#define _STORAGE_CLIENT_H_

#include "network/fasttransport.hpp"
#include "network/configuration.hpp"
#include "common/node_id.hpp"
#include "common/transaction.hpp"

class StorageClient : public network::TransportReceiver
{
  public:
    StorageClient(const network::Configuration &config,
             network::Transport *transport,
             uint64_t clientid = 0);
    virtual ~StorageClient();

    // All RPCs this client can invoke
    virtual bool UpsertNode(uint8_t coreIdx, NodeID node_id, const string &serialized_node_buffer);
    virtual std::string GetNode(uint8_t coreIdx, NodeID node_id);
    virtual bool Lock(uint8_t coreIdx, const Transaction &txn);
    //virtual bool Validate(uint8_t coreIdx, const string &txn);
    //virtual bool Commit(uint8_t coreIdx, const string &txn);
    //virtual bool Abort(uint8_t coreIdx, const string &txn);

    // Inherited from TransportReceiver
    void ReceiveRequest(uint8_t reqType, char *reqBuf, char *respBuf) override { PPanic("Not implemented."); };
    void ReceiveResponse(uint8_t reqType, char *respBuf) override;
    bool Blocked() override { return blocked; };

protected:

    network::Configuration config;
    network::Transport *transport;
    uint64_t lastReqId;

    // We assume this client is single-threaded and synchronous

    uint64_t clientid;
    bool blocked;

    // Handlers for replies to the RPC calls
    void HandleGetNodeReply(char *respBuf);
    void HandleUpsertNodeReply(char *respBuf);
    void HandleLockReply(char *respBuf);
    //void HandleValidateReply(char *respBuf);
    //void HandleCommitReply(char *respBuf);
    //void HandleAbortReply(char *respBuf);
    bool upsertNodeReply;
    bool lockReply;
    //bool validateReply;
    //bool commitReply;
    //bool abortReply;
    std::string getNodeReply;
};

#endif  /* _STORAGE_CLIENT_H_ */
