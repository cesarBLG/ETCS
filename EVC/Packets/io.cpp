
#include "types.h"
static unsigned char base_64_dec[256];
const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bit_manipulator::bit_manipulator(std::string base64) : position(0)
{
    write_mode = false;
    bits.reserve(base64.size()*3/4);
    
}
std::string bit_manipulator::to_base64()
{
    std::vector<unsigned char> encoded_data;
    for (int i = 0; i < bits.size();) {

        uint32_t octet_a = i < bits.size() ? (unsigned char)bits[i++] : 0;
        uint32_t octet_b = i < bits.size() ? (unsigned char)bits[i++] : 0;
        uint32_t octet_c = i < bits.size() ? (unsigned char)bits[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data.push_back(base64_table[(triple >> 3 * 6) & 0x3F]);
        encoded_data.push_back(base64_table[(triple >> 2 * 6) & 0x3F]);
        encoded_data.push_back(base64_table[(triple >> 1 * 6) & 0x3F]);
        encoded_data.push_back(base64_table[(triple >> 0 * 6) & 0x3F]);
    }
    int mod_table[] = {0, 2, 1};
    int size = 4 * ((bits.size() + 2) / 3);
    while(encoded_data.size() < size)
        encoded_data.push_back('=');
    for (int i = 0; i < mod_table[bits.size() % 3]; i++)
        encoded_data[size - 1 - i] = '=';

    return std::string(encoded_data.begin(), encoded_data.end());
}
void init_io()
{
    for (int i=0; i<64; i++)
    {
        base_64_dec[base64_table[i]]=i;
    }
	base_64_dec['='] = 0;
}