#define FUSE_USE_VERSION 26

#include <fuse/fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "tosfs.h"

static void *fs_addr = NULL;
static size_t fs_size = 0;
static struct tosfs_superblock *sb = NULL;

/* ---- PARTIE STEP1 ---- */
static int futosfs_open(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); return -1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return -1; }

    fs_size = st.st_size;
    fs_addr = mmap(NULL, fs_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fs_addr == MAP_FAILED) { perror("mmap"); close(fd); return -1; }
    close(fd);

    sb = (struct tosfs_superblock *) fs_addr;

    if (sb->magic != TOSFS_MAGIC) {
        fprintf(stderr, "Erreur : fichier invalide (magic=0x%x)\n", sb->magic);
        return -1;
    }

    printf("=== SUPERBLOCK ===\n");
    printf("Magic number : 0x%x\n", sb->magic);
    printf("Block size   : %u\n", sb->block_size);
    printf("Nb blocks    : %u\n", sb->blocks);
    printf("Nb inodes    : %u\n", sb->inodes);
    printf("Root inode   : %u\n", sb->root_inode);
    printf("Bitmap blocks: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->block_bitmap));
    printf("Bitmap inodes: "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->inode_bitmap));

    return 0;
}


/* Vide */
static struct fuse_lowlevel_ops futosfs_ops = {
    // .getattr = futosfs_getattr,
    // .readdir = futosfs_readdir,
    // .lookup  = futosfs_lookup,
};

/* ---- MAIN basé sur hello_ll.c ---- */
int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fichier> <point_de_montage>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    char *mountpoint;
    struct fuse_args args = FUSE_ARGS_INIT(argc-1, argv+1); 

    if (futosfs_open(image_path) != 0) {
        return 1;
    }

    struct fuse_chan *ch;
    int err = -1;

    if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
        (ch = fuse_mount(mountpoint, &args)) != NULL) {
        struct fuse_session *se;

        se = fuse_lowlevel_new(&args, &futosfs_ops, sizeof(futosfs_ops), NULL);
        if (se != NULL) {
            if (fuse_set_signal_handlers(se) != -1) {
                fuse_session_add_chan(se, ch);
                err = fuse_session_loop(se);
                fuse_remove_signal_handlers(se);
                fuse_session_remove_chan(ch);
            }
            fuse_session_destroy(se);
        }
        fuse_unmount(mountpoint, ch);
    }
    fuse_opt_free_args(&args);

    if (fs_addr) munmap(fs_addr, fs_size);

    return err ? 1 : 0;
}
