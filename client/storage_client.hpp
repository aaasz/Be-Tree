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
    virtual bool EvictNode(uint8_t coreIdx, uint32_t serverIdx, uint64_t node_id, const string &request);
    virtual bool UpsertNode(uint8_t coreIdx, NodeID node_id, const string &serialized_node_buffer);

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
    void HandleUpsertNodeReply(char *respBuf);
    void HandleEvictNodeReply(char *respBuf);
    bool evictNodeReply;
    bool upsertNodeReply;
};

#endif  /* _STORAGE_CLIENT_H_ */
