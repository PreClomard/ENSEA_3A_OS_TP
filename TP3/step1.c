#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "tosfs.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_fs>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }
    printf("Taille du fichier : %ld octets (%.2f Ko)\n", st.st_size, st.st_size / 1024.0);

    void *addr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    struct tosfs_superblock *sb = (struct tosfs_superblock *)addr;
    printf("=== SUPERBLOCK ===\n");
    if (sb->magic != TOSFS_MAGIC) {
        fprintf(stderr, "Erreur : Le fichier n'est pas un filesystem tosfs (magic = 0x%x)\n", sb->magic);
        munmap(addr, st.st_size);
        close(fd);
        return 1;
    }
    printf("Magic number : 0x%x\n", sb->magic);
    printf("Block size   : %u\n", sb->block_size);
    printf("Nb blocks    : %u\n", sb->blocks);
    printf("Nb inodes    : %u\n", sb->inodes);
    printf("Root inode   : %u\n", sb->root_inode);
    printf("Bitmap blocks: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->block_bitmap));
    printf("Bitmap inodes: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->inode_bitmap));

    munmap(addr, st.st_size);
    close(fd);
    return 0;
}
