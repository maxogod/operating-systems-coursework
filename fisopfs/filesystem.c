#include "filesystem.h"

char *global_save_file;

// Persistence operations

void
init_filesystem(filesystem_t *fs, char *save_file)
{
	global_save_file = save_file ? save_file : DEFAULT_SAVE_FILE;

	FILE *file = fopen(global_save_file, "r");
	if (!file) {
		fs->blocks[ROOT_BLOCK].status = OCCUPIED;
		strcpy(fs->blocks[ROOT_BLOCK].dir_path, "");
		strcpy(fs->blocks[ROOT_BLOCK].path, ROOT_DIR);
		fs->blocks[ROOT_BLOCK].id_user = getuid();
		fs->blocks[ROOT_BLOCK].id_group = getgid();
		fs->blocks[ROOT_BLOCK].mode = MODE_DIR;
		fs->blocks[ROOT_BLOCK].nlink = NLINK_DIR;
		fs->blocks[ROOT_BLOCK].size = 0;
		return;
	}

	if (fread(fs, sizeof(filesystem_t), 1, file) != 1) {
		fprintf(stderr,
		        "[debug] init_filesystem - error: %s\n",
		        strerror(errno));
		fclose(file);
		return;
	}

	fclose(file);
}

int
save_filesystem(filesystem_t *fs)
{
	FILE *file = fopen(global_save_file, "w");
	if (!file) {
		fprintf(stderr,
		        "[debug] save_filesystem - error: %s\n",
		        strerror(errno));
		return EXIT_FAILURE;
	}

	if (fwrite(fs, sizeof(filesystem_t), 1, file) != 1) {
		fprintf(stderr,
		        "[debug] save_filesystem - error: %s\n",
		        strerror(errno));
		fclose(file);
		return EXIT_FAILURE;
	}

	fflush(file);  // Force write to disk
	fclose(file);
	return EXIT_SUCCESS;
}

// Block operations

block_t *
get_block(filesystem_t *fs, const char *path)
{
	if (strcmp(ROOT_DIR, path) == 0) {
		return &fs->blocks[ROOT_BLOCK];
	}

	for (size_t i = FIRST_BLOCK; i < MAX_BLOCKS; i++) {
		if (fs->blocks[i].status == FREE) {
			continue;
		}
		if (strcmp(fs->blocks[i].path, path) == 0) {
			return &fs->blocks[i];
		}
	}

	return NULL;
}

size_t
get_id_block(filesystem_t *fs, const char *path)
{
	if (strcmp(ROOT_DIR, path) == 0) {
		return ROOT_BLOCK;
	}

	for (size_t i = FIRST_BLOCK; i < MAX_BLOCKS; i++) {
		if (fs->blocks[i].status == FREE) {
			continue;
		}
		if (strcmp(fs->blocks[i].path, path) == 0) {
			return i;
		}
	}

	return -1;
}

block_t *
get_block_free(filesystem_t *fs)
{
	for (size_t i = FIRST_BLOCK; i < MAX_BLOCKS; i++) {
		if (fs->blocks[i].status == FREE) {
			return &fs->blocks[i];
		}
	}

	return NULL;
}

void
set_block_free(filesystem_t *fs, size_t id_block)
{
	if (id_block >= MAX_BLOCKS || id_block < FIRST_BLOCK)
		return;

	fs->blocks[id_block].status = FREE;
	strcpy(fs->blocks[id_block].dir_path, "");
	strcpy(fs->blocks[id_block].path, "");
	strcpy(fs->blocks[id_block].content, "");
}

// Directory operations

int
list_directory(filesystem_t *fs, const char *dir_path, size_t prev_block)
{
	size_t i = prev_block + 1;
	while (i < MAX_BLOCKS) {
		if (strcmp(fs->blocks[i].dir_path, dir_path) == 0)
			return i;
		i++;
	}
	return -1;
}

int
create_directory(filesystem_t *fs, const char *path)
{
	block_t *blk = get_block(fs, path);
	if (blk && blk->type == FS_FILE) {
		return -ENOTDIR;
	}

	if (blk && blk->type == FS_DIR) {
		return -EEXIST;
	}

	block_t *free_blk = get_block_free(fs);
	char *dir_path = get_dir_path(path);

	free_blk->status = OCCUPIED;
	free_blk->type = FS_DIR;
	strcpy(free_blk->dir_path, dir_path);
	strcpy(free_blk->path, path);
	free_blk->mode = MODE_DIR;
	free_blk->nlink = NLINK_DIR;
	free_blk->id_user = getuid();
	free_blk->id_group = getgid();
	free_blk->size = 0;

	free(dir_path);

	return 0;
}

int
remove_directory(filesystem_t *fs, const char *path)
{
	size_t id_blk = get_id_block(fs, path);
	if (id_blk < 0) {
		return -ENOENT;
	}

	block_t blk = fs->blocks[id_blk];

	if (blk.type == FS_FILE) {
		remove_file(fs, blk.path);
	}

	int i = list_directory(fs, path, ROOT_BLOCK);
	while (i > 0) {
		remove_directory(fs, fs->blocks[i].path);
		i = list_directory(fs, path, i);
	}

	set_block_free(fs, id_blk);

	return 0;
}

// File operations

int
create_file(filesystem_t *fs, const char *path)
{
	block_t *blk = get_block(fs, path);
	if (blk && blk->type == FS_FILE) {
		return -EEXIST;
	}

	if (blk && blk->type == FS_DIR) {
		return -EISDIR;
	}

	block_t *free_blk = get_block_free(fs);
	char *dir_path = get_dir_path(path);

	free_blk->status = OCCUPIED;
	free_blk->type = FS_FILE;
	strcpy(free_blk->dir_path, dir_path);
	strcpy(free_blk->path, path);
	free_blk->mode = MODE_FILE;
	free_blk->nlink = NLINK_FILE;
	free_blk->id_user = getuid();
	free_blk->id_group = getgid();
	free_blk->size = 0;
	free_blk->access_time = time(NULL);
	free_blk->modify_time = time(NULL);

	free(dir_path);

	return 0;
}

int
remove_file(filesystem_t *fs, const char *path)
{
	size_t id_blk = get_id_block(fs, path);
	if (id_blk < 0) {
		return -ENOENT;
	}

	if (fs->blocks[id_blk].type == FS_DIR) {
		return -EISDIR;
	}

	// Decrement nlink and remove block if nlink is 0
	fs->blocks[id_blk].nlink--;
	if (fs->blocks[id_blk].nlink <= 0) {
		set_block_free(fs, id_blk);
	}

	return 0;
}

int
read_file(filesystem_t *fs, const char *path, char *buffer, size_t size, off_t offset)
{
	block_t *blk = get_block(fs, path);
	if (!blk) {
		return -ENOENT;
	}

	if (blk->type == FS_DIR) {
		return -EISDIR;
	}

	if (offset + size > strlen(blk->content)) {
		size = strlen(blk->content) - offset;
	}

	size = size > 0 ? size : 0;

	memcpy(buffer, blk->content + offset, size);

	return size;
}

int
write_file(filesystem_t *fs,
           const char *path,
           const char *buffer,
           size_t size,
           off_t offset)
{
	if (offset + size > MAX_CONTENT) {
		return -EFBIG;
	}

	block_t *blk = get_block(fs, path);
	if (!blk) {
		return -ENOENT;
	}

	if (blk->type == FS_DIR) {
		return -EISDIR;
	}

	memcpy(blk->content + offset, buffer, size);
	blk->size += size;
	blk->modify_time = time(NULL);

	return size;
}

int
truncate_file(filesystem_t *fs, const char *path, off_t offset)
{
	if (offset > MAX_CONTENT) {
		return -EFBIG;
	}

	block_t *blk = get_block(fs, path);
	if (!blk) {
		return -ENOENT;
	}

	if (blk->type == FS_DIR) {
		return -EISDIR;
	}

	blk->content[offset] = '\0';
	blk->size = offset;

	return 0;
}

// Miscellaneous operations

int
get_stats(filesystem_t *fs, const char *path, struct stat *st)
{
	block_t *blk = get_block(fs, path);
	if (!blk) {
		return -ENOENT;
	}

	st->st_mode = blk->mode;
	st->st_nlink = blk->nlink;
	st->st_uid = blk->id_user;
	st->st_gid = blk->id_group;
	st->st_size = blk->size;
	st->st_atime = time(NULL);
	st->st_mtime = blk->modify_time;

	return 0;
}

int
update_times(filesystem_t *fs, const char *path)
{
	block_t *blk = get_block(fs, path);
	if (!blk) {
		return -ENOENT;
	}

	blk->access_time = time(NULL);

	return 0;
}

// Helper functions

char *
get_dir_path(const char *path)
{
	char *result = (char *) malloc(sizeof(char) * MAX_PATHNAME);
	int last_slash = strlen(strrchr(path, '/'));
	strcpy(result, path);
	result[strlen(result) - last_slash] = '\0';

	if (strlen(result) == 0)
		strcpy(result, ROOT_DIR);

	return result;
}
