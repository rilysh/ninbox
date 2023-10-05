#ifndef NINBOX_H
#define NINBOX_H    1

#define DEFAULT_CONFIG_DIR    ".ninbox"

#ifdef ENABLE_COLOR
#define RED    "\033[1;91m"
#define GREEN  "\033[1;92m"
#define YELLOW "\033[1;93m"
#define WHITE  "\033[1;97m"
#define END    "\033[0m"

#define __err(x)  (RED x END)
#define __ok(x)   (GREEN x END)
#else
#define RED
#define GREEN
#define YELLOW
#define WHITE
#define END

#define __err(x)    (x)
#define __ok(x)     (x)
#endif

#define args_iterate_over(fn)						\
	do {								\
		for (i = 2; i < argc; i++) {				\
			if (argv[i][0] == '-' && argv[i][1] == '-') {	\
				fputs(__err("error: possibly mutiple arguments were tried to be passed, " \
					    "however, it's not allowed!\n"), stderr); \
				exit(EXIT_FAILURE);			\
			}						\
			fn;						\
		}							\
	} while (0)

#define single_check_multiple_args(i)					\
	do {								\
		if (argv[i][0] == '-' && argv[i][1] == '-') {		\
			fputs(__err("error: possibly mutiple arguments were tried to be passed, " \
				    "however, it's not allowed!\n"), stderr); \
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

enum {
	BLOCK     = 0,  NON_BLOCK = 1,  LOAD_NAME = 1,
	LOAD_NUM  = 2,  EDIT_NAME = 3,  EDIT_NUM  = 4,
	DEL_NAME  = 5,  DEL_NUM   = 6,  LIST      = 7,
	ROOT_PATH = 8,  ROM_PATH  = 9,  SETTINGS  = 10,
	VIEW_NAME = 11, VIEW_NUM  = 12, USAGE     = 13
};

struct long_opts {
	int load_name_opt;
	int load_num_opt;
	int edit_name_opt;
	int edit_num_opt;
	int del_name_opt;
	int del_num_opt;
	int list_opt;
	int root_path_opt;
	int rom_path_opt;
	int settings_opt;
	int view_name_opt;
	int view_num_opt;
};

#endif
