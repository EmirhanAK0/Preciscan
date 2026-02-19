#pragma once
#include <string>
#include "config.h"

bool write_meta_json(
    const std::string& path,
    const Config& cfg
);
