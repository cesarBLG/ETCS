
#include "../types.h"
#include "base64.h"
bit_manipulator::bit_manipulator(std::string base64) : position(0)
{
    write_mode = false;
    std::string str = base64_decode(base64);
    bits.insert(bits.end(), str.begin(), str.end());
}
std::string bit_manipulator::to_base64()
{
    return base64_encode(&bits[0], bits.size());
}