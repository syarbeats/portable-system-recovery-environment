#include <stdint.h>
#include <stddef.h>
#include <psre/m003/cpu.h>

static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *a, uint32_t *b,
                  uint32_t *c, uint32_t *d) {
    __asm__ volatile("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(leaf), "c"(subleaf));
}

void psre_cpu_detect(psre_cpu_info_t *info) {
    if (!info) return;
    for (size_t i = 0; i < sizeof(*info); ++i) ((uint8_t *)info)[i] = 0;

    uint32_t a, b, c, d;
    cpuid(0, 0, &a, &b, &c, &d);
    info->max_basic_leaf = a;
    ((uint32_t *)info->vendor)[0] = b;
    ((uint32_t *)info->vendor)[1] = d;
    ((uint32_t *)info->vendor)[2] = c;
    info->vendor[12] = '\0';

    cpuid(0x80000000u, 0, &a, &b, &c, &d);
    info->max_extended_leaf = a;

    if (info->max_extended_leaf >= 0x80000004u) {
        uint32_t *brand = (uint32_t *)info->brand;
        for (uint32_t leaf = 0; leaf < 3; ++leaf) {
            cpuid(0x80000002u + leaf, 0,
                  &brand[leaf * 4 + 0], &brand[leaf * 4 + 1],
                  &brand[leaf * 4 + 2], &brand[leaf * 4 + 3]);
        }
        info->brand[48] = '\0';
    }

    if (info->max_basic_leaf >= 1) {
        cpuid(1, 0, &a, &b, &c, &d);
        info->stepping = a & 0xF;
        uint32_t base_model = (a >> 4) & 0xF;
        uint32_t base_family = (a >> 8) & 0xF;
        uint32_t ext_model = (a >> 16) & 0xF;
        uint32_t ext_family = (a >> 20) & 0xFF;
        info->family = (base_family == 0xF) ? base_family + ext_family : base_family;
        info->model = (base_family == 0x6 || base_family == 0xF)
                    ? (ext_model << 4) | base_model : base_model;
        info->has_apic = (uint8_t)((d >> 9) & 1);
        info->has_msr = (uint8_t)((d >> 5) & 1);
        info->has_sse = (uint8_t)((d >> 25) & 1);
        info->has_sse2 = (uint8_t)((d >> 26) & 1);
        info->has_x2apic = (uint8_t)((c >> 21) & 1);
    }
}
