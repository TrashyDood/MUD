#pragma once

#include <cstddef>
#include <vector>
#include <queue>

using NetId = int;
using NetPriority = unsigned short;
// typedef int (*func)(int, int);

class NetObj;

class NetRegistry
{
    friend class NetObj;

    std::vector<NetObj *> netObjects;
    std::queue<NetId> vacantIds;

    void Register(NetObj &netObj);
    void Unregister(NetObj &netObj);

public:
    NetRegistry() = default;
    NetObj *Find(NetId id) const;
};

inline NetRegistry StaticNetRegistry;