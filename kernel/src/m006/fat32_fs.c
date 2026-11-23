#include <psre/m006/fs.h>
#include <psre/m005/storage.h>
#include <psre/m047/block.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

#define SECTOR_SIZE 512u
#define ATTR_READ_ONLY 0x01u
#define ATTR_HIDDEN    0x02u
#define ATTR_SYSTEM    0x04u
#define ATTR_VOLUME_ID 0x08u
#define ATTR_DIRECTORY 0x10u
#define ATTR_ARCHIVE   0x20u
#define ATTR_LFN      0x0Fu
#define FAT32_EOC_MIN 0x0FFFFFF8u
#define FAT32_BAD     0x0FFFFFF7u
#define FAT32_RESERVED_MIN 0x0FFFFFF0u

typedef enum {
    FSERR_NONE = 0, FSERR_NOT_MOUNTED, FSERR_BAD_ROOT_CLUSTER,
    FSERR_CLUSTER_RANGE, FSERR_CLUSTER_LBA, FSERR_DATA_READ,
    FSERR_FAT_READ, FSERR_BAD_CHAIN, FSERR_CHAIN_LOOP
} fs_error_t;

static psre_fat32_volume_t mounted;
static bool is_mounted;
static bool mounted_usb;
static uint32_t max_data_cluster;
static fs_error_t last_error;
static uint64_t last_error_lba;
static uint32_t last_error_cluster;
static uint8_t io_sector[SECTOR_SIZE] __attribute__((aligned(16)));
static uint8_t cluster_sector[SECTOR_SIZE] __attribute__((aligned(16)));

static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void write_dec(uint64_t x) {
    char b[21]; unsigned n = 0;
    if (!x) { serial_write("0"); return; }
    while (x && n < sizeof(b)) { b[n++] = (char)('0' + x % 10); x /= 10; }
    while (n) serial_write_char(b[--n]);
}
static void set_error(fs_error_t e, uint64_t lba, uint32_t cluster) {
    last_error = e; last_error_lba = lba; last_error_cluster = cluster;
}
static bool read_rel(uint64_t rel, uint8_t *buf) {
    if (!is_mounted || rel >= mounted.total_sectors ||
        mounted.partition_lba > UINT64_MAX - rel) {
        set_error(FSERR_CLUSTER_LBA, mounted.partition_lba, 0);
        return false;
    }
    uint64_t absolute = mounted.partition_lba + rel;
    if (mounted_usb) {
        const psre_m047_status_t *u=psre_m047_get_status();
        if(!u||absolute>=u->sectors||!psre_m047_read_sectors(absolute,1,buf)){
            set_error(FSERR_DATA_READ,absolute,0);return false;
        }
    } else {
        const psre_ata_device_t *ds=psre_storage_get_devices();
        if(mounted.device_index>=PSRE_STORAGE_MAX_DEVICES||absolute>=ds[mounted.device_index].sectors){
            set_error(FSERR_CLUSTER_LBA,absolute,0);return false;
        }
        if(!psre_ata_read_sector(mounted.device_index,absolute,buf)){set_error(FSERR_DATA_READ,absolute,0);return false;}
    }
    return true;
}
static bool cluster_lba(uint32_t cluster, uint64_t *lba) {
    if (!lba || cluster < 2 || cluster > max_data_cluster) {
        set_error(FSERR_CLUSTER_RANGE, 0, cluster);
        return false;
    }
    uint64_t data_start = (uint64_t)mounted.reserved_sectors +
                          (uint64_t)mounted.fat_count * mounted.sectors_per_fat;
    uint64_t rel = data_start +
                   (uint64_t)(cluster - 2) * mounted.sectors_per_cluster;
    if (rel > mounted.total_sectors ||
        mounted.sectors_per_cluster > mounted.total_sectors - rel) {
        set_error(FSERR_CLUSTER_LBA, mounted.partition_lba + rel, cluster);
        return false;
    }
    *lba = rel;
    return true;
}
static bool read_fat_entry(uint32_t cluster, uint32_t *next) {
    if (!next || cluster < 2 || cluster > max_data_cluster) {
        set_error(FSERR_CLUSTER_RANGE, 0, cluster);
        return false;
    }
    uint64_t byte_off = (uint64_t)cluster * 4u;
    uint64_t fat_rel = mounted.reserved_sectors + byte_off / SECTOR_SIZE;
    if (byte_off / SECTOR_SIZE >= mounted.sectors_per_fat) {
        set_error(FSERR_BAD_CHAIN, mounted.partition_lba + fat_rel, cluster);
        return false;
    }
    if (!read_rel(fat_rel, io_sector)) {
        set_error(FSERR_FAT_READ, mounted.partition_lba + fat_rel, cluster);
        return false;
    }
    *next = rd32(&io_sector[byte_off % SECTOR_SIZE]) & 0x0FFFFFFFu;
    return true;
}
/* Returns 1 for next cluster, 0 for EOC, -1 for invalid/error. */
static int get_next_cluster(uint32_t current, uint32_t *next) {
    if (!read_fat_entry(current, next)) return -1;
    if (*next >= FAT32_EOC_MIN) return 0;
    if (*next == 0 || *next == 1 || *next == FAT32_BAD ||
        *next >= FAT32_RESERVED_MIN || *next > max_data_cluster) {
        set_error(FSERR_BAD_CHAIN, 0, current);
        return -1;
    }
    return 1;
}
static void format_short_name(const uint8_t *e, char out[14]) {
    unsigned n = 0;
    for (unsigned i = 0; i < 8 && e[i] != ' '; ++i)
        out[n++] = (char)e[i];
    if (e[8] != ' ') {
        out[n++] = '.';
        for (unsigned i = 8; i < 11 && e[i] != ' '; ++i)
            out[n++] = (char)e[i];
    }
    out[n] = 0;
}
static bool name_equal(const char *a, const char *b) {
    while (*a && *b) {
        char x = *a++, y = *b++;
        if (x >= 'a' && x <= 'z') x -= 32;
        if (y >= 'a' && y <= 'z') y -= 32;
        if (x != y) return false;
    }
    return *a == 0 && *b == 0;
}
static void report_error(void) {
    switch (last_error) {
        case FSERR_NOT_MOUNTED: serial_write("[FAT32] Error: volume is not mounted.\n"); break;
        case FSERR_BAD_ROOT_CLUSTER: serial_write("[FAT32] Error: BPB root cluster is invalid.\n"); break;
        case FSERR_CLUSTER_RANGE: serial_write("[FAT32] Error: cluster is outside the data region. Cluster="); write_dec(last_error_cluster); serial_write("\n"); break;
        case FSERR_CLUSTER_LBA:
            serial_write("[FAT32] Error: cluster LBA is outside volume. Cluster=");
            write_dec(last_error_cluster); serial_write(" LBA="); write_dec(last_error_lba); serial_write("\n"); break;
        case FSERR_DATA_READ:
            serial_write("[FAT32] Error: ATA sector read failed. LBA="); write_dec(last_error_lba); serial_write("\n"); break;
        case FSERR_FAT_READ:
            serial_write("[FAT32] Error: FAT sector read failed. LBA="); write_dec(last_error_lba); serial_write("\n"); break;
        case FSERR_BAD_CHAIN:
            serial_write("[FAT32] Error: invalid FAT cluster-chain entry. Cluster="); write_dec(last_error_cluster); serial_write("\n"); break;
        case FSERR_CHAIN_LOOP: serial_write("[FAT32] Error: cluster-chain traversal limit reached.\n"); break;
        default: serial_write("[FAT32] Error: unspecified filesystem error.\n"); break;
    }
}
bool psre_fat32_mount_first(void) {
    mounted_usb = false;
    is_mounted = false;
    max_data_cluster = 0;
    last_error = FSERR_NONE;
    const psre_ata_device_t *ds = psre_storage_get_devices();
    for (uint8_t d = 0; d < PSRE_STORAGE_MAX_DEVICES; ++d) {
        if (!ds[d].present) continue;
        psre_fat32_volume_t v = {0};
        if (psre_fat32_probe(d, 0, &v)) {
            mounted = v; is_mounted = true;
        } else {
            if (!psre_ata_read_sector(d, 0, io_sector) ||
                io_sector[510] != 0x55 || io_sector[511] != 0xAA) continue;
            for (unsigned p = 0; p < 4 && !is_mounted; ++p) {
                const uint8_t *e = &io_sector[446 + p * 16];
                uint8_t type = e[4]; uint32_t start = rd32(&e[8]);
                if (!start || !(type == 0x0B || type == 0x0C ||
                                type == 0x1B || type == 0x1C)) continue;
                if (psre_fat32_probe(d, start, &v)) {
                    mounted = v; is_mounted = true;
                }
            }
        }
        if (is_mounted) break;
    }
    if (!is_mounted) {
        serial_write("[M006] No mountable FAT32 volume found.\n");
        return false;
    }
    uint64_t overhead = (uint64_t)mounted.reserved_sectors +
                        (uint64_t)mounted.fat_count * mounted.sectors_per_fat;
    if (overhead >= mounted.total_sectors) {
        is_mounted = false;
        serial_write("[M006] Invalid FAT32 layout: metadata exceeds volume.\n");
        return false;
    }
    uint64_t data_sectors = mounted.total_sectors - overhead;
    uint64_t data_clusters = data_sectors / mounted.sectors_per_cluster;
    if (data_clusters < 1 || data_clusters > 0x0FFFFFEEu) {
        is_mounted = false;
        serial_write("[M006] Invalid FAT32 data-cluster count.\n");
        return false;
    }
    max_data_cluster = (uint32_t)data_clusters + 1u;
    if (mounted.root_cluster < 2 || mounted.root_cluster > max_data_cluster) {
        is_mounted = false;
        serial_write("[M006] Invalid FAT32 root cluster: ");
        write_dec(mounted.root_cluster); serial_write("\n");
        return false;
    }
    /* Ensure the FAT has enough entries for every data cluster. */
    if ((uint64_t)mounted.sectors_per_fat * (SECTOR_SIZE / 4u) <
        (uint64_t)max_data_cluster + 1u) {
        is_mounted = false;
        serial_write("[M006] Invalid FAT32 layout: FAT is too small.\n");
        return false;
    }
    serial_write("[M006] FAT32 mounted read-only: device ");
    write_dec(mounted.device_index); serial_write(", partition LBA ");
    write_dec(mounted.partition_lba); serial_write(", root cluster ");
    write_dec(mounted.root_cluster); serial_write(", max cluster ");
    write_dec(max_data_cluster); serial_write("\n");
    return true;
}

bool psre_fat32_mount_usb(void) {
 is_mounted=false;mounted_usb=false;max_data_cluster=0;last_error=FSERR_NONE;
 const psre_m047_status_t *u=psre_m047_get_status();
 if(!u||!u->ready||u->block_size!=512u){serial_write("[M047] USB block device is not ready.\\n");return false;}
 psre_fat32_volume_t v={0};
 if(psre_fat32_probe_usb(0,&v)){mounted=v;is_mounted=true;}
 else if(psre_m047_read_sectors(0,1,io_sector)&&io_sector[510]==0x55&&io_sector[511]==0xAA){
  for(unsigned p=0;p<4&&!is_mounted;p++){const uint8_t *e=&io_sector[446+p*16];uint8_t t=e[4];uint32_t start=rd32(&e[8]);
   if(!start||!(t==0x0B||t==0x0C||t==0x1B||t==0x1C))continue;
   if(psre_fat32_probe_usb(start,&v)){mounted=v;is_mounted=true;}
  }
 }
 if(!is_mounted){serial_write("[M047] No FAT32 volume found on USB.\\n");return false;}
 uint64_t overhead=(uint64_t)mounted.reserved_sectors+(uint64_t)mounted.fat_count*mounted.sectors_per_fat;
 if(overhead>=mounted.total_sectors){serial_write("[M047] Invalid USB FAT32 layout.\\n");return false;}
 uint64_t clusters=(mounted.total_sectors-overhead)/mounted.sectors_per_cluster;
 if(!clusters||clusters>0x0FFFFFEEu){serial_write("[M047] Invalid USB FAT32 cluster count.\\n");return false;}
 max_data_cluster=(uint32_t)clusters+1u;
 if(mounted.root_cluster<2||mounted.root_cluster>max_data_cluster||
    (uint64_t)mounted.sectors_per_fat*128u<(uint64_t)max_data_cluster+1u){serial_write("[M047] Invalid USB FAT32 geometry.\\n");return false;}
 mounted_usb=true;is_mounted=true;(void)psre_m007_change_dir("/");serial_write("[M047] USB FAT32 mounted read-only; partition LBA ");
 write_dec(mounted.partition_lba);serial_write("\\n");return true;
}
bool psre_fat32_is_mounted(void) { return is_mounted; }
const psre_fat32_volume_t *psre_fat32_get_mounted(void) {
    return is_mounted ? &mounted : 0;
}
bool psre_fat32_list_root(void) {
    last_error = FSERR_NONE;
    if (!is_mounted) { set_error(FSERR_NOT_MOUNTED, 0, 0); report_error(); return false; }
    uint32_t c = mounted.root_cluster;
    uint64_t max_steps = (uint64_t)max_data_cluster + 1u;
    serial_write("Type       Size       Name\n");
    for (uint64_t guard = 0; guard < max_steps; ++guard) {
        uint64_t first;
        if (!cluster_lba(c, &first)) { report_error(); return false; }
        for (uint32_t s = 0; s < mounted.sectors_per_cluster; ++s) {
            if (!read_rel(first + s, cluster_sector)) { report_error(); return false; }
            for (unsigned off = 0; off < SECTOR_SIZE; off += 32) {
                const uint8_t *e = &cluster_sector[off];
                if (e[0] == 0x00) return true;
                if (e[0] == 0xE5 || e[11] == ATTR_LFN ||
                    (e[11] & ATTR_VOLUME_ID) || e[0] == '.') continue;
                serial_write((e[11] & ATTR_DIRECTORY) ? "DIR        " : "FILE       ");
                write_dec(rd32(&e[28])); serial_write("  ");
                char name[14]; format_short_name(e, name);
                serial_write(name); serial_write("\n");
            }
        }
        uint32_t n = 0;
        int rc = get_next_cluster(c, &n);
        if (rc == 0) return true;
        if (rc < 0) { report_error(); return false; }
        c = n;
    }
    set_error(FSERR_CHAIN_LOOP, 0, c); report_error();
    return false;
}
bool psre_fat32_cat_root(const char *short_name) {
    last_error = FSERR_NONE;
    if (!is_mounted) { set_error(FSERR_NOT_MOUNTED, 0, 0); report_error(); return false; }
    if (!short_name || !*short_name) return false;
    uint32_t c = mounted.root_cluster, file_cluster = 0, file_size = 0;
    uint64_t max_steps = (uint64_t)max_data_cluster + 1u;
    bool found = false;
    for (uint64_t guard = 0; guard < max_steps && !found; ++guard) {
        uint64_t first;
        if (!cluster_lba(c, &first)) { report_error(); return false; }
        for (uint32_t s = 0; s < mounted.sectors_per_cluster && !found; ++s) {
            if (!read_rel(first + s, cluster_sector)) { report_error(); return false; }
            for (unsigned off = 0; off < SECTOR_SIZE; off += 32) {
                const uint8_t *e = &cluster_sector[off];
                if (e[0] == 0x00) break;
                if (e[0] == 0xE5 || e[11] == ATTR_LFN ||
                    (e[11] & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) || e[0] == '.') continue;
                char name[14]; format_short_name(e, name);
                if (name_equal(name, short_name)) {
                    file_cluster = ((uint32_t)rd16(&e[20]) << 16) | rd16(&e[26]);
                    file_size = rd32(&e[28]); found = true; break;
                }
            }
        }
        if (!found) {
            uint32_t n = 0; int rc = get_next_cluster(c, &n);
            if (rc == 0) break;
            if (rc < 0) { report_error(); return false; }
            c = n;
        }
    }
    if (!found) { serial_write("[FAT32] File not found in root (8.3 names only).\n"); return false; }
    if (file_size == 0) { serial_write("[empty file]\n"); return true; }
    if (file_cluster < 2 || file_cluster > max_data_cluster) {
        set_error(FSERR_CLUSTER_RANGE, 0, file_cluster); report_error(); return false;
    }
    uint32_t remaining = file_size;
    c = file_cluster;
    for (uint64_t guard = 0; remaining && guard < max_steps; ++guard) {
        uint64_t first;
        if (!cluster_lba(c, &first)) { report_error(); return false; }
        for (uint32_t s = 0; s < mounted.sectors_per_cluster && remaining; ++s) {
            if (!read_rel(first + s, io_sector)) { report_error(); return false; }
            uint32_t take = remaining < SECTOR_SIZE ? remaining : SECTOR_SIZE;
            for (uint32_t i = 0; i < take; ++i) {
                unsigned char ch = io_sector[i];
                if (ch == '\n') serial_write("\n");
                else if (ch == '\r' || ch == '\t' || (ch >= 0x20 && ch < 0x7F))
                    serial_write_char((char)ch);
                else serial_write_char('.');
            }
            remaining -= take;
        }
        if (remaining) {
            uint32_t n = 0; int rc = get_next_cluster(c, &n);
            if (rc <= 0) {
                if (rc == 0) serial_write("[FAT32] File chain ended before file size.\n");
                else report_error();
                return false;
            }
            c = n;
        }
    }
    if (remaining) { set_error(FSERR_CHAIN_LOOP, 0, c); report_error(); return false; }
    serial_write("\n");
    return true;
}


/* M007: path-aware, read-only FAT32 operations. Long File Names are not yet supported. */
static uint32_t cwd_cluster;
static char cwd_path[PSRE_FS_MAX_PATH] = "/";
static bool m007_ready;

static bool find_entry_in_dir(uint32_t dir_cluster, const char *name, uint8_t out[32]) {
    uint32_t c = dir_cluster;
    uint64_t max_steps = (uint64_t)max_data_cluster + 1u;
    for (uint64_t guard = 0; guard < max_steps; ++guard) {
        uint64_t first;
        if (!cluster_lba(c, &first)) return false;
        for (uint32_t sec = 0; sec < mounted.sectors_per_cluster; ++sec) {
            if (!read_rel(first + sec, cluster_sector)) return false;
            for (unsigned off = 0; off < SECTOR_SIZE; off += 32) {
                const uint8_t *e = &cluster_sector[off];
                if (e[0] == 0x00) return false;
                if (e[0] == 0xE5 || e[11] == ATTR_LFN ||
                    (e[11] & ATTR_VOLUME_ID)) continue;
                char short_name[14];
                format_short_name(e, short_name);
                if (name_equal(short_name, name)) {
                    for (unsigned i = 0; i < 32; ++i) out[i] = e[i];
                    return true;
                }
            }
        }
        uint32_t next = 0;
        int rc = get_next_cluster(c, &next);
        if (rc <= 0) return false;
        c = next;
    }
    set_error(FSERR_CHAIN_LOOP, 0, c);
    return false;
}
static bool resolve_dir_path(const char *path, uint32_t *cluster_out,
                             char *normalized, size_t normalized_size) {
    if (!is_mounted || !path || !cluster_out || !normalized || normalized_size < 2)
        return false;
    uint32_t current = (path[0] == '/') ? mounted.root_cluster :
                      (m007_ready ? cwd_cluster : mounted.root_cluster);
    char work[PSRE_FS_MAX_PATH];
    size_t len = 0;
    while (path[len] && len < sizeof(work) - 1) { work[len] = path[len]; ++len; }
    if (path[len]) return false;
    work[len] = 0;
    size_t pos = 0;
    while (pos < len) {
        while (work[pos] == '/') ++pos;
        if (pos >= len) break;
        size_t start = pos;
        while (work[pos] && work[pos] != '/') ++pos;
        char saved = work[pos]; work[pos] = 0;
        char *part = &work[start];
        if (name_equal(part, ".")) {
            /* no-op */
        } else if (name_equal(part, "..")) {
            if (current != mounted.root_cluster) {
                uint8_t e[32];
                if (find_entry_in_dir(current, "..", e)) {
                    uint32_t parent = ((uint32_t)rd16(&e[20]) << 16) | rd16(&e[26]);
                    if (parent >= 2 && parent <= max_data_cluster) current = parent;
                    else current = mounted.root_cluster;
                } else current = mounted.root_cluster;
            }
        } else {
            uint8_t e[32];
            if (!find_entry_in_dir(current, part, e) || !(e[11] & ATTR_DIRECTORY)) {
                work[pos] = saved;
                return false;
            }
            uint32_t next = ((uint32_t)rd16(&e[20]) << 16) | rd16(&e[26]);
            if (next < 2 || next > max_data_cluster) {
                set_error(FSERR_CLUSTER_RANGE, 0, next);
                work[pos] = saved;
                return false;
            }
            current = next;
        }
        work[pos] = saved;
    }
    *cluster_out = current;
    /* Canonical display path is maintained by the shell, not reconstructed here. */
    normalized[0] = 0;
    return true;
}
static bool find_path_entry(const char *path, uint8_t out[32]) {
    if (!path || !*path || !is_mounted) return false;
    char work[PSRE_FS_MAX_PATH];
    size_t len = 0;
    while (path[len] && len < sizeof(work)-1) { work[len] = path[len]; ++len; }
    if (path[len]) return false;
    work[len] = 0;
    while (len && work[len-1] == '/') work[--len] = 0;
    if (!len) return false;
    char *last = work + len;
    while (last > work && last[-1] != '/') --last;
    char leaf[PSRE_FS_NAME_MAX];
    size_t leaf_len = len - (size_t)(last - work);
    if (!leaf_len || leaf_len >= sizeof(leaf)) return false;
    for (size_t i=0;i<leaf_len;i++) leaf[i]=last[i];
    leaf[leaf_len]=0;
    uint32_t parent;
    if (last == work) parent = (work[0] == '/') ? mounted.root_cluster :
                               (m007_ready ? cwd_cluster : mounted.root_cluster);
    else {
        char parent_path[PSRE_FS_MAX_PATH];
        size_t plen = (size_t)(last - work);
        if (plen && work[plen-1] == '/') --plen;
        for (size_t i=0;i<plen;i++) parent_path[i]=work[i];
        parent_path[plen]=0;
        char ignored[2];
        if (!resolve_dir_path(parent_path, &parent, ignored, sizeof(ignored))) return false;
    }
    return find_entry_in_dir(parent, leaf, out);
}
static void print_entry(uint8_t e[32]) {
    char name[14]; format_short_name(e, name);
    serial_write((e[11] & ATTR_DIRECTORY) ? "DIR  " : "FILE ");
    serial_write(" ");
    write_dec(rd32(&e[28]));
    serial_write("  "); serial_write(name); serial_write("\n");
}
bool psre_m007_change_dir(const char *path) {
    uint32_t target;
    char ignored[2];
    last_error = FSERR_NONE;
    if (!resolve_dir_path(path && *path ? path : "/", &target, ignored, sizeof(ignored))) {
        serial_write("[M007] Directory not found or invalid 8.3 path.\n");
        if (last_error != FSERR_NONE) report_error();
        return false;
    }
    char display[PSRE_FS_MAX_PATH];
    size_t dlen = 0;
    if (path && *path && path[0] != '/' && m007_ready) {
        while (cwd_path[dlen] && dlen < sizeof(display)-1) {
            display[dlen] = cwd_path[dlen]; ++dlen;
        }
    } else display[dlen++] = '/';
    if (dlen == 0) display[dlen++] = '/';
    size_t pos = 0;
    while (path && path[pos]) {
        while (path[pos] == '/') ++pos;
        if (!path[pos]) break;
        size_t start = pos;
        while (path[pos] && path[pos] != '/') ++pos;
        size_t part_len = pos - start;
        if (part_len == 1 && path[start] == '.') continue;
        if (part_len == 2 && path[start] == '.' && path[start+1] == '.') {
            while (dlen > 1 && display[dlen-1] == '/') --dlen;
            while (dlen > 1 && display[dlen-1] != '/') --dlen;
            if (dlen > 1) --dlen;
            if (dlen == 0) display[dlen++] = '/';
            continue;
        }
        if (dlen > 1 && display[dlen-1] != '/') display[dlen++] = '/';
        if (dlen + part_len >= sizeof(display)) {
            serial_write("[M007] Display path exceeds maximum length.\n");
            return false;
        }
        for (size_t i=0; i<part_len; ++i) display[dlen++] = path[start+i];
    }
    if (dlen > 1 && display[dlen-1] == '/') --dlen;
    display[dlen] = 0;
    for (size_t i=0; i<=dlen; ++i) cwd_path[i] = display[i];
    cwd_cluster = target;
    m007_ready = true;
    return true;
}
const char *psre_m007_get_cwd(void) {
    if (!m007_ready) { cwd_cluster = mounted.root_cluster; cwd_path[0]='/'; cwd_path[1]=0; m007_ready=true; }
    return cwd_path;
}
bool psre_m007_list_dir(const char *path) {
    if (!is_mounted) { serial_write("[M007] No mounted filesystem.\n"); return false; }
    uint32_t dir = cwd_cluster;
    char ignored[2];
    if (path && *path && !resolve_dir_path(path,&dir,ignored,sizeof(ignored))) {
        serial_write("[M007] Directory not found.\n"); report_error(); return false;
    }
    uint32_t c=dir;
    uint64_t max_steps=(uint64_t)max_data_cluster+1u;
    serial_write("Type  Size       Name\n");
    for(uint64_t guard=0;guard<max_steps;guard++) {
        uint64_t first;
        if(!cluster_lba(c,&first)){report_error();return false;}
        for(uint32_t sec=0;sec<mounted.sectors_per_cluster;sec++) {
            if(!read_rel(first+sec,cluster_sector)){report_error();return false;}
            for(unsigned off=0;off<SECTOR_SIZE;off+=32) {
                uint8_t *e=&cluster_sector[off];
                if(e[0]==0x00)return true;
                if(e[0]==0xE5||e[11]==ATTR_LFN||(e[11]&ATTR_VOLUME_ID))continue;
                print_entry(e);
            }
        }
        uint32_t next=0; int rc=get_next_cluster(c,&next);
        if(rc==0)return true;
        if(rc<0){report_error();return false;}
        c=next;
    }
    set_error(FSERR_CHAIN_LOOP,0,c);report_error();return false;
}

bool psre_m007_read_dir(const char *path, psre_m007_dir_entry_t *entries,
                        uint32_t capacity, uint32_t *count, uint32_t *total) {
    if (!count || !total || (capacity && !entries) || !is_mounted) return false;
    *count = 0; *total = 0; last_error = FSERR_NONE;
    uint32_t dir = cwd_cluster;
    char ignored[2];
    if (path && *path && !resolve_dir_path(path, &dir, ignored, sizeof(ignored))) {
        if (last_error != FSERR_NONE) report_error();
        return false;
    }
    uint32_t c = dir;
    uint64_t max_steps = (uint64_t)max_data_cluster + 1u;
    for (uint64_t guard = 0; guard < max_steps; ++guard) {
        uint64_t first;
        if (!cluster_lba(c, &first)) { report_error(); return false; }
        for (uint32_t sec = 0; sec < mounted.sectors_per_cluster; ++sec) {
            if (!read_rel(first + sec, cluster_sector)) { report_error(); return false; }
            for (unsigned off = 0; off < SECTOR_SIZE; off += 32) {
                const uint8_t *e = &cluster_sector[off];
                if (e[0] == 0x00) return true;
                if (e[0] == 0xE5 || e[11] == ATTR_LFN ||
                    (e[11] & ATTR_VOLUME_ID) || e[0] == '.') continue;
                if (*total < capacity) {
                    psre_m007_dir_entry_t *out = &entries[*total];
                    format_short_name(e, out->name);
                    out->size = rd32(&e[28]);
                    out->first_cluster = ((uint32_t)rd16(&e[20]) << 16) | rd16(&e[26]);
                    out->attributes = e[11];
                    ++*count;
                }
                ++*total;
            }
        }
        uint32_t next = 0;
        int rc = get_next_cluster(c, &next);
        if (rc == 0) return true;
        if (rc < 0) { report_error(); return false; }
        c = next;
    }
    set_error(FSERR_CHAIN_LOOP, 0, c); report_error();
    return false;
}

bool psre_m007_stat(const char *path) {
    uint8_t e[32];
    if(!find_path_entry(path,e)){serial_write("[M007] Entry not found (8.3 path required).\n");if(last_error!=FSERR_NONE)report_error();return false;}
    serial_write("Name: ");char name[14];format_short_name(e,name);serial_write(name);
    serial_write("\nType: ");serial_write((e[11]&ATTR_DIRECTORY)?"Directory":"File");
    serial_write("\nSize: ");write_dec(rd32(&e[28]));
    serial_write("\nAttributes: ");write_dec(e[11]);
    serial_write("\nFirst cluster: ");write_dec(((uint32_t)rd16(&e[20])<<16)|rd16(&e[26]));
    serial_write("\n");return true;
}
bool psre_m007_crc32(const char *path) {
    uint8_t e[32];
    if(!find_path_entry(path,e)){serial_write("[M007] File not found (8.3 path required).\n");if(last_error!=FSERR_NONE)report_error();return false;}
    if(e[11]&ATTR_DIRECTORY){serial_write("[M007] CRC32 requires a file, not a directory.\n");return false;}
    uint32_t size=rd32(&e[28]);
    uint32_t c=((uint32_t)rd16(&e[20])<<16)|rd16(&e[26]);
    if(size && (c<2||c>max_data_cluster)){set_error(FSERR_CLUSTER_RANGE,0,c);report_error();return false;}
    uint32_t crc=0xFFFFFFFFu,remaining=size;
    uint64_t max_steps=(uint64_t)max_data_cluster+1u;
    for(uint64_t guard=0;remaining&&guard<max_steps;guard++){
        uint64_t first;if(!cluster_lba(c,&first)){report_error();return false;}
        for(uint32_t sec=0;sec<mounted.sectors_per_cluster&&remaining;sec++){
            if(!read_rel(first+sec,io_sector)){report_error();return false;}
            uint32_t take=remaining<SECTOR_SIZE?remaining:SECTOR_SIZE;
            for(uint32_t i=0;i<take;i++){
                crc^=io_sector[i];
                for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^(0xEDB88320u & (uint32_t)-(int32_t)(crc&1u));
            }
            remaining-=take;
        }
        if(remaining){uint32_t next=0;int rc=get_next_cluster(c,&next);if(rc<=0){if(rc==0)serial_write("[M007] File chain ended before file size.\n");else report_error();return false;}c=next;}
    }
    if(remaining){set_error(FSERR_CHAIN_LOOP,0,c);report_error();return false;}
    crc^=0xFFFFFFFFu;
    static const char hex[]="0123456789ABCDEF";
    serial_write("CRC32: 0x");
    for(int shift=28;shift>=0;shift-=4)serial_write_char(hex[(crc>>shift)&15]);
    serial_write("\n");return true;
}
bool psre_m007_cat_path(const char *path) {
    uint8_t e[32];
    if(!find_path_entry(path,e)){serial_write("[M007] File not found (8.3 path required).\n");if(last_error!=FSERR_NONE)report_error();return false;}
    if(e[11]&ATTR_DIRECTORY){serial_write("[M007] Cannot cat a directory.\n");return false;}
    uint32_t size=rd32(&e[28]),c=((uint32_t)rd16(&e[20])<<16)|rd16(&e[26]);
    if(!size){serial_write("[empty file]\n");return true;}
    if(c<2||c>max_data_cluster){set_error(FSERR_CLUSTER_RANGE,0,c);report_error();return false;}
    uint32_t remaining=size;uint64_t max_steps=(uint64_t)max_data_cluster+1u;
    for(uint64_t guard=0;remaining&&guard<max_steps;guard++){
        uint64_t first;if(!cluster_lba(c,&first)){report_error();return false;}
        for(uint32_t sec=0;sec<mounted.sectors_per_cluster&&remaining;sec++){
            if(!read_rel(first+sec,io_sector)){report_error();return false;}
            uint32_t take=remaining<SECTOR_SIZE?remaining:SECTOR_SIZE;
            for(uint32_t i=0;i<take;i++){unsigned char ch=io_sector[i];if(ch=='\n')serial_write("\n");else if(ch=='\r'||ch=='\t'||(ch>=0x20&&ch<0x7f))serial_write_char((char)ch);else serial_write_char('.');}
            remaining-=take;
        }
        if(remaining){uint32_t next=0;int rc=get_next_cluster(c,&next);if(rc<=0){if(rc==0)serial_write("[M007] File chain ended before file size.\n");else report_error();return false;}c=next;}
    }
    serial_write("\n");return remaining==0;
}


/* M008: bounded RAM recovery workspace. No ATA write path is used. */
#include <psre/heap.h>
#define M008_MAX_FILE_BYTES (128u * 1024u)
static uint8_t *m008_data;
static uint32_t m008_size;
static uint32_t m008_crc32;
static uint8_t m008_sha256[32];
static char m008_source[PSRE_FS_MAX_PATH];
static uint64_t m008_session_id;
static bool m008_valid;

typedef struct { uint32_t h[8]; uint64_t bits; uint8_t block[64]; uint32_t used; } sha256_ctx;
static const uint32_t sha_k[64] = {
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
static uint32_t ror32(uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));}
static void sha_transform(sha256_ctx *c,const uint8_t *b){
 uint32_t w[64]; for(unsigned i=0;i<16;i++) w[i]=((uint32_t)b[i*4]<<24)|((uint32_t)b[i*4+1]<<16)|((uint32_t)b[i*4+2]<<8)|b[i*4+3];
 for(unsigned i=16;i<64;i++){uint32_t a=ror32(w[i-15],7)^ror32(w[i-15],18)^(w[i-15]>>3),d=ror32(w[i-2],17)^ror32(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+a+w[i-7]+d;}
 uint32_t a=c->h[0],bb=c->h[1],cc=c->h[2],d=c->h[3],e=c->h[4],f=c->h[5],g=c->h[6],h=c->h[7];
 for(unsigned i=0;i<64;i++){uint32_t s1=ror32(e,6)^ror32(e,11)^ror32(e,25),ch=(e&f)^(~e&g),t1=h+s1+ch+sha_k[i]+w[i],s0=ror32(a,2)^ror32(a,13)^ror32(a,22),maj=(a&bb)^(a&cc)^(bb&cc),t2=s0+maj;h=g;g=f;f=e;e=d+t1;d=cc;cc=bb;bb=a;a=t1+t2;}
 c->h[0]+=a;c->h[1]+=bb;c->h[2]+=cc;c->h[3]+=d;c->h[4]+=e;c->h[5]+=f;c->h[6]+=g;c->h[7]+=h;
}
static void sha_init(sha256_ctx*c){static const uint32_t iv[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};for(unsigned i=0;i<8;i++)c->h[i]=iv[i];c->bits=0;c->used=0;}
static void sha_update(sha256_ctx*c,const uint8_t*p,uint32_t n){c->bits+=(uint64_t)n*8;while(n){uint32_t take=64-c->used;if(take>n)take=n;for(uint32_t i=0;i<take;i++)c->block[c->used+i]=p[i];c->used+=take;p+=take;n-=take;if(c->used==64){sha_transform(c,c->block);c->used=0;}}}
static void sha_final(sha256_ctx*c,uint8_t out[32]){uint64_t bits=c->bits;c->block[c->used++]=0x80;if(c->used>56){while(c->used<64)c->block[c->used++]=0;sha_transform(c,c->block);c->used=0;}while(c->used<56)c->block[c->used++]=0;for(unsigned i=0;i<8;i++)c->block[63-i]=(uint8_t)(bits>>(i*8));sha_transform(c,c->block);for(unsigned i=0;i<8;i++){out[i*4]=(uint8_t)(c->h[i]>>24);out[i*4+1]=(uint8_t)(c->h[i]>>16);out[i*4+2]=(uint8_t)(c->h[i]>>8);out[i*4+3]=(uint8_t)c->h[i];}}
static void m008_hex(const uint8_t *p,uint32_t n){static const char h[]="0123456789ABCDEF";for(uint32_t i=0;i<n;i++){serial_write_char(h[p[i]>>4]);serial_write_char(h[p[i]&15]);}}
void psre_m008_clear(void){if(m008_data)kfree(m008_data);m008_data=0;m008_size=0;m008_valid=false;m008_source[0]=0;serial_write("[M008] RAM workspace cleared.\n");}
bool psre_m008_recover(const char *path){
 if(!is_mounted){serial_write("[M008] Mount a FAT32 volume first.\n");return false;}
 uint8_t e[32];last_error=FSERR_NONE;
 if(!find_path_entry(path,e)){serial_write("[M008] Source file not found (FAT 8.3 path required).\n");if(last_error!=FSERR_NONE)report_error();return false;}
 if(e[11]&ATTR_DIRECTORY){serial_write("[M008] Directories cannot be staged.\n");return false;}
 uint32_t size=rd32(&e[28]),c=((uint32_t)rd16(&e[20])<<16)|rd16(&e[26]);
 if(size>M008_MAX_FILE_BYTES){serial_write("[M008] File exceeds 128 KiB workspace limit.\n");return false;}
 if(size && (c<2||c>max_data_cluster)){set_error(FSERR_CLUSTER_RANGE,0,c);report_error();return false;}
 uint8_t *buf=size?(uint8_t*)kmalloc(size):0;if(size&&!buf){serial_write("[M008] RAM allocation failed; clear workspace or choose smaller file.\n");return false;}
 uint32_t remain=size,offset=0;uint64_t max_steps=(uint64_t)max_data_cluster+1u;
 for(uint64_t guard=0;remain&&guard<max_steps;guard++){
  uint64_t first;if(!cluster_lba(c,&first)){if(buf)kfree(buf);report_error();return false;}
  for(uint32_t sec=0;sec<mounted.sectors_per_cluster&&remain;sec++){
   if(!read_rel(first+sec,io_sector)){if(buf)kfree(buf);report_error();return false;}
   uint32_t take=remain<SECTOR_SIZE?remain:SECTOR_SIZE;for(uint32_t i=0;i<take;i++)buf[offset+i]=io_sector[i];offset+=take;remain-=take;
  }
  if(remain){uint32_t next=0;int rc=get_next_cluster(c,&next);if(rc<=0){if(buf)kfree(buf);if(rc==0)serial_write("[M008] FAT chain shorter than file size.\n");else report_error();return false;}c=next;}
 }
 if(remain){if(buf)kfree(buf);set_error(FSERR_CHAIN_LOOP,0,c);report_error();return false;}
 if(m008_data) { kfree(m008_data); }
 m008_data=buf; m008_size=size;
 uint32_t crc=0xFFFFFFFFu;sha256_ctx sh;sha_init(&sh);
 if(size)sha_update(&sh,buf,size);
 for(uint32_t i=0;i<size;i++){crc^=buf[i];for(unsigned b=0;b<8;b++)crc=(crc>>1)^(0xEDB88320u&(uint32_t)-(int32_t)(crc&1u));}
 m008_crc32=crc^0xFFFFFFFFu;sha_final(&sh,m008_sha256);
 size_t i=0;while(path[i]&&i<sizeof(m008_source)-1){m008_source[i]=path[i];i++;}m008_source[i]=0;
 m008_session_id++;m008_valid=true;
 serial_write("[M008] File staged in RAM. Bytes=");write_dec(size);serial_write(" CRC32=0x");
 m008_hex((uint8_t*)&m008_crc32,0); /* print CRC in network order below */
 {static const char h[]="0123456789ABCDEF";for(int shf=28;shf>=0;shf-=4)serial_write_char(h[(m008_crc32>>shf)&15]);}
 serial_write("\n[M008] SHA256=");m008_hex(m008_sha256,32);serial_write("\n");return true;
}
void psre_m008_workspace_info(void){
 serial_write("[M008] Workspace: ");serial_write(m008_valid?"READY\n":"EMPTY\n");
 if(m008_valid){serial_write("  Source: ");serial_write(m008_source);serial_write("\n  Size: ");write_dec(m008_size);serial_write(" bytes\n  Session: ");write_dec(m008_session_id);serial_write("\n");}
 serial_write("  Maximum file size: 131072 bytes\n  Heap free: ");write_dec(heap_get_free_bytes());serial_write(" bytes\n");
}
void psre_m008_manifest(void){
 if(!m008_valid){serial_write("[M008] No recovery session. Use recover PATH.\n");return;}
 serial_write("PSRE-RECOVERY-MANIFEST/1\nSession: ");write_dec(m008_session_id);
 serial_write("\nSource: ");serial_write(m008_source);serial_write("\nSize: ");write_dec(m008_size);
 serial_write("\nCRC32: ");{static const char h[]="0123456789ABCDEF";for(int shf=28;shf>=0;shf-=4)serial_write_char(h[(m008_crc32>>shf)&15]);}
 serial_write("\nSHA256: ");m008_hex(m008_sha256,32);serial_write("\nStorage: RAM workspace\nSource disk modified: NO\n");
}
bool psre_m008_export_serial(void){
 if(!m008_valid){serial_write("[M008] No staged file to export.\n");return false;}
 serial_write("PSRE-HEX-EXPORT/1\nSIZE:");write_dec(m008_size);serial_write("\nDATA:\n");
 for(uint32_t off=0;off<m008_size;off+=32){uint32_t n=m008_size-off;if(n>32)n=32;serial_write("D:");m008_hex(m008_data+off,n);serial_write("\n");}
 serial_write("END:");m008_hex(m008_sha256,32);serial_write("\n");
 serial_write("[M008] Serial HEX export complete. Capture terminal output and decode D: records.\n");return true;
}


/* M009: bounded multi-file recovery queue, backed by a dedicated RAM pool. */
#define M009_MAX_FILES 16u
#define M009_POOL_BYTES (128u * 1024u)
typedef struct {
    char path[PSRE_FS_MAX_PATH];
    uint32_t offset, size, crc32;
    uint8_t sha256[32];
} m009_item_t;
static uint8_t m009_pool[M009_POOL_BYTES] __attribute__((aligned(16)));
static m009_item_t m009_items[M009_MAX_FILES];
static uint32_t m009_count, m009_used;
static void m009_write_hex(const uint8_t *p,uint32_t n) {
    static const char h[]="0123456789ABCDEF";
    for(uint32_t i=0;i<n;i++){serial_write_char(h[p[i]>>4]);serial_write_char(h[p[i]&15]);}
}
bool psre_m009_queue_add(const char *path) {
    if(!path||!*path){serial_write("[M009] Usage: queue-add PATH\n");return false;}
    if(m009_count>=M009_MAX_FILES){serial_write("[M009] Queue full (16 files).\n");return false;}
    if(!psre_m008_recover(path))return false;
    if(m008_size>M009_POOL_BYTES-m009_used){serial_write("[M009] RAM queue capacity exceeded (128 KiB total).\n");return false;}
    m009_item_t *it=&m009_items[m009_count];
    size_t n=0;while(m008_source[n]&&n<sizeof(it->path)-1){it->path[n]=m008_source[n];n++;}it->path[n]=0;
    it->offset=m009_used;it->size=m008_size;it->crc32=m008_crc32;
    for(unsigned i=0;i<32;i++)it->sha256[i]=m008_sha256[i];
    for(uint32_t i=0;i<m008_size;i++)m009_pool[m009_used+i]=m008_data[i];
    m009_used+=m008_size;m009_count++;
    serial_write("[M009] Queued ");serial_write(it->path);serial_write(" (");write_dec(it->size);serial_write(" bytes). Queue ");
    write_dec(m009_count);serial_write("/16, RAM ");write_dec(m009_used);serial_write("/131072 bytes.\n");
    return true;
}
void psre_m009_queue_list(void) {
    serial_write("[M009] Recovery queue: ");write_dec(m009_count);serial_write("/16 files, ");
    write_dec(m009_used);serial_write("/131072 bytes.\n");
    for(uint32_t i=0;i<m009_count;i++){serial_write("  ");write_dec(i+1);serial_write(". ");
      serial_write(m009_items[i].path);serial_write("  ");write_dec(m009_items[i].size);serial_write(" bytes\n");}
}
void psre_m009_queue_manifest(void) {
    serial_write("PSRE-MULTI-RECOVERY-MANIFEST/1\nFiles: ");write_dec(m009_count);
    serial_write("\nTotal bytes: ");write_dec(m009_used);serial_write("\nSource disk modified: NO\n");
    for(uint32_t i=0;i<m009_count;i++){m009_item_t *it=&m009_items[i];serial_write("FILE ");write_dec(i+1);
      serial_write("\nPath: ");serial_write(it->path);serial_write("\nOffset: ");write_dec(it->offset);
      serial_write("\nSize: ");write_dec(it->size);serial_write("\nCRC32: ");
      {static const char h[]="0123456789ABCDEF";for(int shf=28;shf>=0;shf-=4)serial_write_char(h[(it->crc32>>shf)&15]);}
      serial_write("\nSHA256: ");m009_write_hex(it->sha256,32);serial_write("\n");}
}
bool psre_m009_queue_export(void) {
    if(!m009_count){serial_write("[M009] Queue is empty.\n");return false;}
    serial_write("PSRE-MULTI-HEX-EXPORT/1\nFILES:");write_dec(m009_count);serial_write("\n");
    for(uint32_t i=0;i<m009_count;i++){m009_item_t *it=&m009_items[i];
      serial_write("BEGIN:");write_dec(i+1);serial_write(":");serial_write(it->path);serial_write(":");write_dec(it->size);serial_write("\n");
      for(uint32_t off=0;off<it->size;off+=32){uint32_t n=it->size-off;if(n>32)n=32;
        serial_write("D:");m009_write_hex(&m009_pool[it->offset+off],n);serial_write("\n");}
      serial_write("END:");m009_write_hex(it->sha256,32);serial_write("\n");
      serial_write("PROGRESS:");write_dec(i+1);serial_write("/");write_dec(m009_count);serial_write("\n");
    }
    serial_write("PSRE-EXPORT-COMPLETE\n[M009] Capture serial output; decode each file's D: records separately.\n");
    return true;
}
void psre_m009_queue_clear(void) {
    psre_m009_export_cancel();
    for(uint32_t i=0;i<M009_MAX_FILES;i++){m009_items[i].path[0]=0;m009_items[i].size=0;}
    m009_count=0;m009_used=0;serial_write("[M009] Multi-file queue cleared.\n");
}


/* M009 read-only queue inspection API for the M042 GUI. */
bool psre_m009_queue_get_info(uint32_t *count, uint32_t *used_bytes) {
    if (!count || !used_bytes) return false;
    *count = m009_count;
    *used_bytes = m009_used;
    return true;
}
bool psre_m009_queue_get_entry(uint32_t index, psre_m009_queue_entry_t *out) {
    if (!out || index >= m009_count) return false;
    m009_item_t *it = &m009_items[index];
    size_t n = 0;
    while (it->path[n] && n + 1u < sizeof(out->path)) {
        out->path[n] = it->path[n]; ++n;
    }
    out->path[n] = 0;
    out->size = it->size;
    out->crc32 = it->crc32;
    return true;
}

/* M043: cooperative M009 serial export. Each step emits at most byte_budget
 * payload bytes (record framing is additional). Calls are single-threaded. */
static bool m009_x_active;
static uint32_t m009_x_file, m009_x_offset, m009_x_done, m009_x_total;
static bool m009_x_end_pending;
bool psre_m009_export_begin(void) {
    if (!m009_count || m009_x_active) return false;
    m009_x_file=0; m009_x_offset=0; m009_x_done=0; m009_x_total=m009_used;
    m009_x_end_pending=false; m009_x_active=true;
    serial_write("PSRE-MULTI-HEX-EXPORT/1\nFILES:"); write_dec(m009_count); serial_write("\n");
    return true;
}
bool psre_m009_export_step(uint32_t budget, uint32_t *done, uint32_t *total, uint32_t *files_done, bool *complete) {
    if (!m009_x_active || !done || !total || !files_done || !complete) return false;
    if (budget < 32u) budget=32u;
    uint32_t emitted=0;
    while (m009_x_file<m009_count && emitted<budget) {
        m009_item_t *it=&m009_items[m009_x_file];
        if (m009_x_offset==0 && !m009_x_end_pending) {
            serial_write("BEGIN:"); write_dec(m009_x_file+1u); serial_write(":");
            serial_write(it->path); serial_write(":"); write_dec(it->size); serial_write("\n");
            m009_x_end_pending=true;
        }
        if (m009_x_offset<it->size) {
            uint32_t n=it->size-m009_x_offset; if(n>32u)n=32u;
            serial_write("D:"); m009_write_hex(&m009_pool[it->offset+m009_x_offset],n); serial_write("\n");
            m009_x_offset+=n; m009_x_done+=n; emitted+=n;
        }
        if (m009_x_offset>=it->size) {
            serial_write("END:"); m009_write_hex(it->sha256,32); serial_write("\nPROGRESS:");
            write_dec(m009_x_file+1u); serial_write("/"); write_dec(m009_count); serial_write("\n");
            ++m009_x_file; m009_x_offset=0; m009_x_end_pending=false;
        }
    }
    *done=m009_x_done; *total=m009_x_total; *files_done=m009_x_file; *complete=(m009_x_file>=m009_count);
    if (*complete) { serial_write("PSRE-EXPORT-COMPLETE\n[M009] Cooperative serial export complete.\n"); m009_x_active=false; }
    return true;
}
void psre_m009_export_cancel(void) { m009_x_active=false; m009_x_file=m009_x_offset=0; }
