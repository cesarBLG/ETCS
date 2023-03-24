#pragma once
#include "../Packets/packets.h"
#include "../Packets/radio.h"
std::shared_ptr<ETCS_packet> translate_packet(std::shared_ptr<ETCS_packet> packet, std::vector<std::shared_ptr<ETCS_packet>> packets, int version);
std::shared_ptr<euroradio_message> translate_message(std::shared_ptr<euroradio_message> message, int version);
std::shared_ptr<euroradio_message_traintotrack> translate_message(std::shared_ptr<euroradio_message_traintotrack> message, int version);