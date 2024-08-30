#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SAVE_FILE "save.fisopfs"

#define MAX_BLOCKS 1024
#define MAX_PATHNAME 256
#define MAX_NAME 256
#define MAX_CONTENT 4096

#define ROOT_DIR "/"
#define CURR_DIR "."
#define PREV_DIR ".."

// mode
#define MODE_DIR __S_IFDIR | 0755
#define MODE_FILE __S_IFREG | 0644

// nlink
#define NLINK_DIR 2
#define NLINK_FILE 1

#define ROOT_BLOCK 0
#define FIRST_BLOCK 1

typedef enum blk_status { FREE, OCCUPIED } blk_status_t;
typedef enum blk_type { FS_DIR, FS_FILE } blk_type_t;

typedef struct __attribute__((__packed__)) block {
	blk_status_t status;          // Status of block (FREE || OCCUPIED)
	blk_type_t type;              // Type of block (FS_DIR || FS_FILE)
	char dir_path[MAX_PATHNAME];  // Dir of file
	char path[MAX_PATHNAME];      // Pathname of file
	char content[MAX_CONTENT];    // Content of file
	mode_t mode;                  // Protection (Read, Written or Executed)
	nlink_t nlink;                // Number of hard links
	uid_t id_user;                // ID of user
	gid_t id_group;               // ID of group
	off_t size;                   // Total size, in bytes
	time_t access_time;           // Time of last access
	time_t modify_time;           // Time of last modification
} block_t;

typedef struct __attribute__((__packed__)) filesystem {
	block_t blocks[MAX_BLOCKS];
} filesystem_t;

// Persistence operations
void init_filesystem(filesystem_t *fs, char *save_file);
int save_filesystem(filesystem_t *fs);

// Block operations
block_t *get_block(filesystem_t *fs, const char *path);
size_t get_id_block(filesystem_t *fs, const char *path);
block_t *get_block_free(filesystem_t *fs);
void set_block_free(filesystem_t *fs, size_t id_block);

// Directory operations
int create_directory(filesystem_t *fs, const char *path);
int remove_directory(filesystem_t *fs, const char *path);
int list_directory(filesystem_t *fs, const char *dir_path, size_t prev_block);

// File operations
int create_file(filesystem_t *fs, const char *path);
int remove_file(filesystem_t *fs, const char *path);
int read_file(filesystem_t *fs,
              const char *path,
              char *buffer,
              size_t size,
              off_t offset);
int write_file(filesystem_t *fs,
               const char *path,
               const char *buffer,
               size_t size,
               off_t offset);
int truncate_file(filesystem_t *fs, const char *path, off_t offset);

// Miscellaneous operations
int get_stats(filesystem_t *fs, const char *path, struct stat *st);
int update_times(filesystem_t *fs, const char *path);

// Helper functions
char *get_dir_path(const char *path);
