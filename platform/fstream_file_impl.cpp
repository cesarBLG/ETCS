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

std::string FstreamFileImpl::read_file(const std::string &path) {
    std::ifstream file(load_path + path, std::ios::binary | std::ios::ate);
    if (!file.good())
        return "";
    size_t size = file.tellg();
    file.seekg(0);
    std::string buffer;
    buffer.resize(size);
    file.read(buffer.data(), size);
    return buffer;
}

void FstreamFileImpl::write_file(const std::string &path, const std::string &contents) {
    std::ofstream file(load_path + path, std::ios::binary);
    if (!file.good())
        return;
    file.write(contents.data(), contents.size());
}
