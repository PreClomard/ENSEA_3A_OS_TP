#define FUSE_USE_VERSION 26

#include <fuse/fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tosfs.h"

// Définitions explicites pour le champ mode des inodes
#define TOSFS_FILE 0
#define TOSFS_DIR  1

// Définitions manuelles des macros car absentes
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#ifndef S_IFREG
#define S_IFREG 0100000
#endif

static void *fs_addr = NULL;
static size_t fs_size = 0;
static struct tosfs_superblock *sb = NULL;

static int futosfs_open(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return -1;
    }

    fs_size = st.st_size;
    fs_addr = mmap(NULL, fs_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fs_addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }
    close(fd);

    sb = (struct tosfs_superblock *)fs_addr;
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

static void futosfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    struct stat st;
    memset(&st, 0, sizeof(st));

    if (ino == FUSE_ROOT_ID) {
        st.st_ino = FUSE_ROOT_ID;
        st.st_mode = S_IFDIR | 0755;
        st.st_nlink = 2;  // "." et ".."
        fuse_reply_attr(req, &st, 1.0);
    }
    else if (ino <= sb->inodes) {
        struct tosfs_inode *inode = (struct tosfs_inode *)
            ((char *)fs_addr + TOSFS_BLOCK_SIZE * TOSFS_INODE_BLOCK + (ino - 1) * sizeof(struct tosfs_inode));
        st.st_ino = inode->inode;
        st.st_mode = (inode->mode == TOSFS_DIR) ? S_IFDIR | inode->perm : S_IFREG | inode->perm;
        st.st_nlink = inode->nlink;
        st.st_size = inode->size;
        st.st_uid = inode->uid;
        st.st_gid = inode->gid;
        st.st_blocks = (inode->size + 511) / 512;
        fuse_reply_attr(req, &st, 1.0);
    }
    else {
        fuse_reply_err(req, ENOENT);
    }
}

static void futosfs_access(fuse_req_t req, fuse_ino_t ino, int mask) {
    fuse_reply_err(req, 0);
}

static void futosfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
    if (parent != FUSE_ROOT_ID) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    if (strncmp(name, ".Trash", 6) == 0) {
        fuse_reply_err(req, ENOENT);
        return;
    }

    struct tosfs_dentry *dentries = (struct tosfs_dentry *)
        ((char *)fs_addr + TOSFS_BLOCK_SIZE * TOSFS_ROOT_BLOCK);

    for (size_t i = 0; i < TOSFS_BLOCK_SIZE / sizeof(struct tosfs_dentry); i++) {
        if (dentries[i].inode == 0) {
            break;
        }
        if (strcmp(dentries[i].name, name) == 0) {
            struct fuse_entry_param e;
            memset(&e, 0, sizeof(e));
            e.ino = dentries[i].inode;
            e.attr_timeout = 1.0;
            e.entry_timeout = 1.0;
            struct tosfs_inode *inode = (struct tosfs_inode *)
                ((char *)fs_addr + TOSFS_BLOCK_SIZE * TOSFS_INODE_BLOCK + (dentries[i].inode - 1) * sizeof(struct tosfs_inode));
            e.attr.st_ino = inode->inode;
            e.attr.st_mode = (inode->mode == TOSFS_DIR) ? S_IFDIR | inode->perm : S_IFREG | inode->perm;
            e.attr.st_nlink = inode->nlink;
            e.attr.st_size = inode->size;
            fuse_reply_entry(req, &e);
            return;
        }
    }
    fuse_reply_err(req, ENOENT);
}

static void futosfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
    if (ino != FUSE_ROOT_ID) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    char *buf = malloc(size);
    if (!buf) {
        fuse_reply_err(req, ENOMEM);
        return;
    }

    struct stat st;
    memset(&st, 0, sizeof(st));
    size_t pos = 0;

    st.st_ino = FUSE_ROOT_ID;
    pos = fuse_add_direntry(req, buf, size, ".", &st, off + 1);
    pos = fuse_add_direntry(req, buf + pos, size - pos, "..", &st, off + 2);

    struct tosfs_dentry *dentries = (struct tosfs_dentry *)
        ((char *)fs_addr + TOSFS_BLOCK_SIZE * TOSFS_ROOT_BLOCK);

    for (size_t i = 0; i < TOSFS_BLOCK_SIZE / sizeof(struct tosfs_dentry); i++) {
        if (dentries[i].inode == 0)
            break;
        struct tosfs_inode *inode = (struct tosfs_inode *)
            ((char *)fs_addr + TOSFS_BLOCK_SIZE * TOSFS_INODE_BLOCK + (dentries[i].inode - 1) * sizeof(struct tosfs_inode));
        memset(&st, 0, sizeof(st));
        st.st_ino = inode->inode;
        st.st_mode = (inode->mode == TOSFS_DIR) ? S_IFDIR | inode->perm : S_IFREG | inode->perm;
        pos = fuse_add_direntry(req, buf + pos, size - pos, dentries[i].name, &st, off + i + 3);
    }

    fuse_reply_buf(req, buf, pos);
    free(buf);
}

static struct fuse_lowlevel_ops futosfs_ops = {
    .getattr = futosfs_getattr,
    .lookup  = futosfs_lookup,
    .readdir = futosfs_readdir,
    .access  = futosfs_access,
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fichier_fs> <point_de_montage>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    if (futosfs_open(image_path) != 0) {
        return 1;
    }

    struct fuse_args args = FUSE_ARGS_INIT(argc - 2, argv + 2);
    char *mountpoint = argv[2];

    struct fuse_chan *ch = fuse_mount(mountpoint, &args);
    if (ch == NULL) {
        fprintf(stderr, "Échec du montage FUSE\n");
        return 1;
    }

    struct fuse_session *se = fuse_lowlevel_new(&args, &futosfs_ops, sizeof(futosfs_ops), NULL);
    if (se == NULL) {
        fprintf(stderr, "Échec de la création de la session FUSE\n");
        fuse_unmount(mountpoint, ch);
        return 1;
    }

    if (fuse_set_signal_handlers(se) == -1) {
        fprintf(stderr, "Échec de la configuration des handlers de signaux\n");
        fuse_session_destroy(se);
        fuse_unmount(mountpoint, ch);
        return 1;
    }

    fuse_session_add_chan(se, ch);

    int err = fuse_session_loop(se);

    fuse_remove_signal_handlers(se);
    fuse_session_remove_chan(ch);
    fuse_session_destroy(se);
    fuse_unmount(mountpoint, ch);

    if (fs_addr) munmap(fs_addr, fs_size);

    return err ? 1 : 0;
}
