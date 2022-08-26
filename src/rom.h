#pragma once
#include "cartridge.h"
#include "nes.h"

namespace nes {

std::unique_ptr<Mapper> LoadRomFile(const std::string &path);
}