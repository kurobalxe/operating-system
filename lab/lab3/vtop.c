// vtop_all.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <string.h>
#include <getopt.h>

static long page_size;

// 通用函数：通过 /proc/pid/pagemap 获取物理地址
uint64_t get_phys_addr(pid_t pid, void *virt_addr) {
    char pagemap_path[64];
    snprintf(pagemap_path, sizeof(pagemap_path), "/proc/%d/pagemap", (int)pid);
    int fd = open(pagemap_path, O_RDONLY);
    if (fd < 0) {
        perror("open pagemap");
        return 0;
    }

    uint64_t virt_page = (uintptr_t)virt_addr / page_size;
    uint64_t entry;
    off_t offset = virt_page * sizeof(entry);

    if (pread(fd, &entry, sizeof(entry), offset) != sizeof(entry)) {
        perror("pread pagemap");
        close(fd);
        return 0;
    }
    close(fd);

    if (!(entry & (1ULL << 63))) {
        // printf("Page not present in RAM.\n");
        return 0;
    }

    uint64_t pfn = entry & ((1ULL << 55) - 1);
    uint64_t page_offset = (uintptr_t)virt_addr % page_size;
    return (pfn << (__builtin_ctzll(page_size))) + page_offset;
}

// 打印地址信息
void print_info(const char *name, void *addr, pid_t pid) {
    uint64_t vaddr = (uint64_t)addr;
    uint64_t phys = get_phys_addr(pid, addr);
    uint64_t vpage = vaddr / page_size;
    uint64_t voffset = vaddr % page_size;
    
    printf("=== %s ===\n", name);
    printf("Symbol name:       %s\n", name);
    printf("Virtual address:   0x%016lx\n", vaddr);
    printf("Virtual page #:    %lu (0x%lx)\n", vpage, vpage);
    printf("Offset in page:    0x%03lx (%lu bytes)\n", voffset, voffset);
    
    if (phys) {
        uint64_t ppage = phys / page_size;
        uint64_t poffset = phys % page_size;
        printf("Physical address:  0x%016lx\n", phys);
        printf("Physical page #:   %lu (0x%lx)\n", ppage, ppage);
        printf("Offset in page:    0x%03lx (%lu bytes)\n", poffset, poffset);
        
        // 验证转换是否正确
        if (voffset != poffset) {
            printf("WARNING: Virtual and physical offsets don't match!\n");
        }
    } else {
        printf("Physical address:  (not in RAM or inaccessible)\n");
    }
    printf("\n");
}

// ========== 全局变量与函数（用于默认模式） ==========
int global_var = 0xCAFEBABE;

void dummy_function(void) {
    // empty
}

// ========== 模式1：固定虚拟地址 mmap ==========
void run_mode1() {
    const void *fixed_virt = (void*)0x10000000; // 固定虚拟地址
    void *mem = mmap((void*)fixed_virt, page_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                     -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap for mode1");
        exit(1);
    }

    pid_t mypid = getpid();
    memcpy(mem, &mypid, sizeof(mypid)); // 写入 PID 作为标识

    printf("[Mode 1] Process %d using fixed virtual address %p\n", mypid, mem);
    printf("Pagemap file: /proc/%d/pagemap\n\n", mypid);
    
    sleep(2); // 留时间给另一个进程启动

    print_info("Fixed mmap region", mem, mypid);
    munmap(mem, page_size);
}

// ========== 模式2：共享库物理地址 ==========
void run_mode2() {
    pid_t mypid = getpid();

    // 强制解析 printf 地址
    void *printf_addr = dlsym(RTLD_DEFAULT, "printf");
    if (!printf_addr) {
        fprintf(stderr, "dlsym failed to find printf\n");
        exit(1);
    }

    // 触发页面加载
    printf("[Mode 2] Hello from PID %d\n", mypid);
    printf("Pagemap file: /proc/%d/pagemap\n\n", mypid);

    print_info("printf (from libc)", printf_addr, mypid);
    sleep(5); // 方便对比
}

// ========== 默认模式 ==========
void run_default() {
    pid_t pid = getpid();
    printf("=== Default Mode: Current Process (%d) ===\n", pid);
    printf("Page size: %ld bytes\n", page_size);
    printf("Pagemap file: /proc/%d/pagemap\n\n", pid);

    print_info("global_var", &global_var, pid);
    print_info("dummy_function", (void*)dummy_function, pid);
}

// ========== 主函数 ==========
void show_usage(char *prog) {
    printf("Usage: %s [-m MODE]\n", prog);
    printf("  -m 1 : Mode 1 - Same virtual address, different physical addresses (run twice)\n");
    printf("  -m 2 : Mode 2 - Shared library (printf) physical address (run twice)\n");
    printf("  (no args): Default mode - show global var and function addresses\n");
}

int main(int argc, char *argv[]) {
    page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf _SC_PAGESIZE");
        return 1;
    }

    int mode = 0;
    int opt;
    while ((opt = getopt(argc, argv, "m:h")) != -1) {
        switch (opt) {
            case 'm':
                mode = atoi(optarg);
                if (mode != 1 && mode != 2) {
                    fprintf(stderr, "Error: mode must be 1 or 2\n");
                    return 1;
                }
                break;
            case 'h':
            default:
                show_usage(argv[0]);
                return 0;
        }
    }

    if (mode == 1) {
        run_mode1();
    } else if (mode == 2) {
        run_mode2();
    } else {
        run_default();
    }

    return 0;
}