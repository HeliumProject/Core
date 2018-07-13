#pragma once

#include "Platform/Types.h"

namespace Helium
{
    std::string MD5(const void* data, uint32_t count);
    std::string MD5(const std::string& data);
    std::string FileMD5(const std::string& filePath, uint32_t packetSize = 4096);
}
