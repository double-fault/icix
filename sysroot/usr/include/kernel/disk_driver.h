#ifndef _KERNEL_DISK_DRIVER_H
#define _KERNEL_DISK_DRIVER_H

typedef struct _disk_driver disk_driver_t; // fwd declaration (https://stackoverflow.com/q/62957620/4688119)

// not the most standard definitions, but works
// init, returns -1 on failure
typedef uint32_t (*init_type_t)(void);
// read, returns bytes read into buf
typedef size_t (*read_type_t)(void*, size_t);
// returns bytes written
typedef size_t (*write_type_t)(void*, size_t);
// SEEK_SET only, returns -1 on fail
typedef uint32_t (*lseek_type_t)(uint32_t);
// returns -1 on fail
typedef uint32_t (*clean_type_t)(void);
// returns -1 on fail
typedef uint32_t (*fsync_type_t)(void);

typedef struct _disk_driver {
	init_type_t init;
	read_type_t read;
	write_type_t write;
	lseek_type_t lseek;
	clean_type_t clean;
	fsync_type_t fsync;
} disk_driver_t;

#endif // _KERNEL_DISK_DRIVER_H
