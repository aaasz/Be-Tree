// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 *   The interface of the store server hosting the nodes of the B3-tree
 *
 **********************************************************************/

#ifndef _STORAGE_SERVER_H_
#define _STORAGE_SERVER_H_

#include "network/configuration.hpp"
#include "network/transport.hpp"
#include "common/node_id.hpp"
#include "common/transaction.hpp"

#include <unordered_map>

class StorageServerApp
{
  public:
    StorageServerApp();
    virtual ~StorageServerApp() { };

    std::string GetNode(NodeID id);
    void UpsertNode(NodeID id, uint16_t size, char* buff);
    bool Lock(Transaction &txn);
    bool Validate(Transaction &txn);
    void Commit(Transaction &txn);
    void Abort(Transaction &txn);

  private:
    uint32_t current_id;

    struct object {
        bool locked;
        Timestamp ts;
        std::string serialized_node;
    };

    std::unordered_map<NodeID, object, node_id_hash_fn> objects;
};

class StorageServer : network::TransportReceiver
{
  public:
    StorageServer(network::Configuration config, int myIdx,
              network::Transport *transport,
              StorageServerApp *storageApp);
    ~StorageServer();

    // Message handlers.
    void ReceiveRequest(uint8_t reqType, char *reqBuf, char *respBuf) override;
    void ReceiveResponse(uint8_t reqType, char *respBuf) override {}; // TODO: for now, servers
                                                                      // do not need to communicate
                                                                      // with eachother
    bool Blocked() override { return false; };
    // new handlers
    void HandleGetNode(char *reqBuf, char *respBuf, size_t &respLen);
    void HandleUpsertNode(char *reqBuf, char *respBuf, size_t &respLen);
    void HandleLock(char *reqBuf, char *respBuf, size_t &respLen);
    void HandleValidate(char *reqBuf, char *respBuf, size_t &respLen);
    void HandleCommit(char *reqBuf, char *respBuf, size_t &respLen);
    void HandleAbort(char *reqBuf, char *respBuf, size_t &respLen);

  private:
    network::Configuration config;
    int myIdx; // Server index into config.
    network::Transport *transport;
    StorageServerApp * storageApp;
};

#endif /* _STORAGE_SERVER_H_ */
