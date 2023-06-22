 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "tcp_socket.h"

class ConsoleFdPoller final : public FdPoller {
	std::vector<std::pair<std::pair<int, short>, PlatformUtil::Fulfiller<short>>> fds;
public:
	virtual PlatformUtil::Promise<short> on_fd_ready(int fd, short ev) override;
	void poll(int timeout);
	bool is_empty();
};
