#pragma once

#include <cstddef>
#include <iostream>
#include <cstdint>
#include <array>
#include <cstring>
#include <stdexcept>
#include <type_traits>

#include "Networking/PacketsIO.h"
#include "ThreadPool.h"
#include "Utils.h"
#include "TickLoop.h"
#include "Networking/NetObj.h"

using Port = unsigned short;
