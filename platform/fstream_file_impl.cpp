 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fstream_file_impl.h"
#include <string>
#include <fstream>

FstreamFileImpl::FstreamFileImpl(const std::string &load_path) : load_path(load_path) {
}

std::optional<std::string> FstreamFileImpl::read_file(const std::string &path) {
    std::ifstream file(load_path + path, std::ios::binary | std::ios::ate);
    if (!file.good())
        return std::nullopt;
    size_t size = file.tellg();
    file.seekg(0);
    std::string buffer;
    buffer.resize(size);
    file.read(buffer.data(), size);
    return buffer;
}

bool FstreamFileImpl::write_file(const std::string &path, const std::string &contents) {
    std::ofstream file(load_path + path, std::ios::binary);
    file.write(contents.data(), contents.size());
    return file.good();
}
