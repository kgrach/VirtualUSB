#pragma once

#include <algorithm>
#include <vector>
// Return memory buffer as a part kernel urb structure
// struct {
//  int             status,
//  unsigned int    actual_length,
//  unsigned char   tranfer_buffer[actual_length]
// }
std::vector<unsigned char> GetResponse(std::vector<unsigned char>& setup_packet);