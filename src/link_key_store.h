#pragma once
#include <cstdint>

struct LinkKeyEntry {
    uint32_t magic;
    uint8_t  bd_addr[6];
    uint8_t  key[16];
    uint8_t  key_type;
    uint8_t  valid;
};

bool lk_load(LinkKeyEntry *out);
bool lk_save(const LinkKeyEntry &entry);
void lk_clear();
