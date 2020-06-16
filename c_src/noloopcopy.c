#include <stdint.h>

uint32_t change_mpk_domain(uint32_t domain);
uint32_t get_mpk_domain();

void mpk_no_loop_copy(void *d, const void *s, uint32_t n) {
    uint32_t original = get_mpk_domain();
    const uint32_t all_memory = 0; // 0b0000
    change_mpk_domain(all_memory);
    asm volatile ("rep movsb\n"
                : "=D" (d),
                  "=S" (s),
                  "=c" (n)
                : "0" (d),
                  "1" (s),
                  "2" (n)
                : "memory");
    change_mpk_domain(original);
}