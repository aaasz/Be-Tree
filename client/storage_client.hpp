// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
#ifndef _STORAGE_CLIENT_H_
#define _STORAGE_CLIENT_H_

#include "network/fasttransport.hpp"
#include "network/configuration.hpp"
#include "common/node_id.hpp"

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
    bool upsertNodeReply;
    std::string getNodeReply;
};

#endif  /* _STORAGE_CLIENT_H_ */
