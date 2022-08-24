#pragma once
#include "mapper.h"
#include "nes.h"

namespace nes {
std::unique_ptr<Mapper> LoadROM(const std::string &path);

}