#pragma once

#include <cstddef>
#include <queue>
#include <vector>

using NetId = int;
using NetPriority = short;

class NetObj;

class NetRegistry
{
    friend class NetObj;

    std::vector<NetObj *> netObjects;
    std::queue<NetId> vacantIds;

    void Register(NetObj &netObject);
    void Unregister(NetObj &netObject);

public:
    NetRegistry() = default;
    NetObj *Find(NetId id) const;
};