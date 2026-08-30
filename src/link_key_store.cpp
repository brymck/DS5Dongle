#include "link_key_store.h"

#include <cstring>
#include <cstdio>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/btstack_flash_bank.h"
#include "pico/flash.h"

#define LK_MAGIC        0x4C4B5354u
#define LK_FLASH_OFFSET PICO_FLASH_BANK_STORAGE_OFFSET

static const LinkKeyEntry *lk_xip() {
    return reinterpret_cast<const LinkKeyEntry *>(XIP_BASE + LK_FLASH_OFFSET);
}

static void lk_flash_op(void *param) {
    const auto *page = static_cast<const uint8_t *>(param);
    const uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(LK_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(LK_FLASH_OFFSET, page, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

bool lk_load(LinkKeyEntry *out) {
    const auto *s = lk_xip();
    if (s->magic != LK_MAGIC || s->valid != 1) return false;
    memcpy(out, s, sizeof(LinkKeyEntry));
    return true;
}

bool lk_save(const LinkKeyEntry &entry) {
    alignas(4) uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0xff, sizeof(page));
    memcpy(page, &entry, sizeof(entry));
    const int rc = flash_safe_execute(lk_flash_op, page, 1000);
    if (rc != PICO_OK) {
        printf("[LK] flash_safe_execute failed: %d\n", rc);
        return false;
    }
    return true;
}

void lk_clear() {
    alignas(4) uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0xff, sizeof(page));
    flash_safe_execute(lk_flash_op, page, 1000);
}
