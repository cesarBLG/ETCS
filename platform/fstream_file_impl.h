 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"

class FstreamFileImpl {
    std::string load_path;

public:
    FstreamFileImpl(const std::string_view load_path);
	std::optional<std::string> read_file(const std::string_view path);
    bool write_file(const std::string_view path, const std::string_view contents);
};
