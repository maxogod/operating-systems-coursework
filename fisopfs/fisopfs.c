#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "filesystem.h"

#define MIN_ARGS 2
#define MAX_ARGS 4

// Global filesystem
filesystem_t filesystem = {};

// Persistence operations

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");
	char *save_file = (char *) fuse_get_context()->private_data;
	init_filesystem(&filesystem, save_file);
	return NULL;
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);
	return save_filesystem(&filesystem);
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy\n");
	save_filesystem(&filesystem);
}

// Directory operations

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	return create_directory(&filesystem, path);
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	return remove_directory(&filesystem, path);
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);
	// Los directorios '.' y '..'
	filler(buffer, CURR_DIR, NULL, 0);
	filler(buffer, PREV_DIR, NULL, 0);

	block_t *blk = get_block(&filesystem, path);
	if (!blk) {
		return -ENOENT;
	}

	if (blk->type == FS_FILE) {
		return -ENOTDIR;
	}

	int i = list_directory(&filesystem, path, ROOT_BLOCK);
	while (i > 0) {
		char *filename = strrchr(filesystem.blocks[i].path, '/') + 1;
		filler(buffer, filename, NULL, 0);
		i = list_directory(&filesystem, path, i);
	}

	return 0;
}

// File operations

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	return create_file(&filesystem, path);
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	return remove_file(&filesystem, path);
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	return read_file(&filesystem, path, buffer, size, offset);
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	return write_file(&filesystem, path, buffer, size, offset);
}

static int
fisopfs_truncate(const char *path, off_t offset)
{
	printf("[debug] fisopfs_truncate - path: %s, offset: %lu\n", path, offset);
	return truncate_file(&filesystem, path, offset);
}

// Miscellaneous operations

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	return get_stats(&filesystem, path, st);
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	return update_times(&filesystem, path);
}

// Fuse

static struct fuse_operations operations = {
	.init = fisopfs_init,
	.flush = fisopfs_flush,
	.destroy = fisopfs_destroy,
	.getattr = fisopfs_getattr,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.readdir = fisopfs_readdir,
	.create = fisopfs_create,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.truncate = fisopfs_truncate,
	.utimens = fisopfs_utimens,
};

int
main(int argc, char *argv[])
{
	if (argc < MIN_ARGS || argc > MAX_ARGS) {
		fprintf(stderr,
		        "Usage: %s (-f) <mountpoint> (<save_file>)\n",
		        argv[0]);
		return EXIT_FAILURE;
	} else if ((argc == 4) || (argc == 3 && strcmp(argv[1], "-f") != 0)) {
		return fuse_main(argc - 1, argv, &operations, argv[argc - 1]);
	}

	return fuse_main(argc, argv, &operations, NULL);
}
