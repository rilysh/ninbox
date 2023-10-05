#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "ninbox.h"

#ifndef BOXPATH
#define BOXPATH    "./boxlaunch.sh"
#endif

static char *trim_conf(char *src)
{
	size_t sz;

	sz = strlen(src);
	/* trim exactly five characters ".conf" */
	src[sz - (size_t)5] = '\0';

	return src;
}

static void nin_exec_cmd(char *cmd, char *sub_cmd, char *p0, char *p1, char *p2, int ignore)
{
	int fd;
	pid_t pid;
	struct stat st;
	char *args[] = { cmd, sub_cmd, p0, p1, p2, NULL };

	if (stat(cmd, &st) == -1) {
		fprintf(stderr, __err(
				"error: program \"%s\" does not exist in respected path.\n"),
			cmd);
		exit(EXIT_FAILURE);
	}

	pid = fork();
	if (pid == (pid_t)-1) {
		perror("fork()");
		exit(EXIT_FAILURE);
	}

	if (pid == (pid_t)0) {
		if (ignore) {
			fd = open("/dev/null", O_WRONLY, 0666);
			if (fd == -1) {
				perror("open()");
				exit(EXIT_FAILURE);
			}

			/* Ignore errors, if duping failed */
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
		}

		if (execv(cmd, args) == -1) {
			perror("execv()");
			exit(EXIT_FAILURE);
		}

		if (ignore)
			while (waitpid(pid, NULL, 0) == -1);
	}

	if (!ignore)
		while (waitpid(pid, NULL, 0) == -1);
}

static char *nin_check_conf_dir(void)
{
	size_t sz;
	char *env, *p;
	struct stat st;

	env = getenv("USER");
	if (env == NULL)
		goto non_extend_error;

	sz = strlen(env) + (size_t)15;
	p = calloc(sz, sizeof(char));
	if (p == NULL)
	        goto non_extend_error;

	sprintf(p, "/home/%s/%s", env, DEFAULT_CONFIG_DIR);

	if (stat(p, &st) == -1) {
		if (mkdir(p, 0777) == -1) {
			free(p);
			goto non_extend_error;
		}
	} else {
		if ((st.st_mode & S_IFMT) != S_IFDIR) {
			/* most obvious error (likely), so we'll notify the user */
			fprintf(stderr, __err("%s is not a directory.\n"), p);
			free(p);
			goto non_extend_error;
		}
	}
	return p;

non_extend_error:
	return strdup("NULL");
}

static char *nin_wrap_conf_path(const char *name)
{
	size_t sz;
	char *conf, *p;

	conf = nin_check_conf_dir();
	if (conf == NULL) {
		fprintf(stderr, __err("error: something went wrong, cannot continue...\n"));
		return NULL;
	}

	sz = strlen(name) + strlen(conf) + (size_t)7;
	p = calloc(sz, sizeof(char));
	if (p == NULL) {
		perror("calloc()");
		free(conf);
		return NULL;
	}

	sprintf(p, "%s/%s.conf", conf, name);
	free(conf);

	return p;
}

static void nin_load_conf(const char *name)
{
	char *conf;

	conf = nin_wrap_conf_path(name);
	if (conf == NULL)
		exit(EXIT_FAILURE);

	nin_exec_cmd(BOXPATH, "-O", conf, NULL, NULL, NON_BLOCK);
	fprintf(stdout,
		__ok("ok: launching \"%s\"\n"), name);
	free(conf);
}

static void nin_load_conf_num(int n)
{
	DIR *dir;
	int i, ok;
        char *conf, *str;
	struct dirent *det;

	if (n <= 0) {
		fprintf(stderr, __err("error: config selection number cannot be <= 0.\n"));
		exit(EXIT_FAILURE);
	}

	i = 1;
	ok = 0;
	conf = nin_check_conf_dir();
	if (conf == NULL) {
		fprintf(stderr, __err("error: something went wrong, cannot continue...\n"));
		exit(EXIT_FAILURE);
	}
	dir = opendir(conf);
	if (dir == NULL) {
		perror("opendir()");
		free(conf);
		exit(EXIT_FAILURE);
	}

	while ((det = readdir(dir)) != NULL) {
		if (det->d_type == DT_REG) {
			str = strrchr(det->d_name, '.');
			if (str != NULL && strcmp(str, ".conf") == 0) {
				if (i == n) {
					ok = 1;
					nin_load_conf(trim_conf(det->d_name));
				        break;
				}
				i++;
			}
		}
	}
	free(conf);
	closedir(dir);

	if (!ok) {
		fprintf(stderr,
			__err("error: config selection number %d was not found. "
			      "maximum config selection number is %d.\n"), n, i - 1);
		exit(EXIT_FAILURE);
	}
}

static void nin_list_conf(void)
{
	int i;
	DIR *dir;
	char *conf, *str;
	struct dirent *det;

	i = 1;
	conf = nin_check_conf_dir();
	if (conf == NULL) {
		fprintf(stderr, __err("error: something went wrong, cannot continue...\n"));
		exit(EXIT_FAILURE);
	}
	dir = opendir(conf);
	if (dir == NULL) {
		perror("opendir()");
		free(conf);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,
		YELLOW"=-=-=-=-=-=-=-=-=-=-\n"END
	        YELLOW"| List of machines |\n"END
	        YELLOW"=-=-=-=-=-=-=-=-=-=-\n"END);

	while ((det = readdir(dir)) != NULL) {
		if (det->d_type == DT_REG) {
			str = strrchr(det->d_name, '.');
			if (str != NULL && strcmp(str, ".conf") == 0) {
				fprintf(stdout, WHITE"%d. %s\n"END, i, trim_conf(det->d_name));
				i++;
			}
		}
	}

	free(conf);
	closedir(dir);
}

static void nin_edit_conf(const char *name)
{
        struct stat st;
	char *conf, *env;

	env = getenv("EDITOR");
	if (env == NULL) {
		fprintf(stderr,
			__err("error: cannot find default editor. "
			      "$EDITOR variable isn't set for current logged in user.\n"));
		exit(EXIT_FAILURE);
	}

	conf = nin_wrap_conf_path(name);
	if (conf == NULL)
		exit(EXIT_FAILURE);

	if (stat(conf, &st) == -1) {
	        fprintf(stderr, __err("error: couldn't find \"%s\" config.\n"), name);
		free(conf);
		exit(EXIT_FAILURE);
	}

        nin_exec_cmd(env, conf, NULL, NULL, NULL, BLOCK);
	free(conf);
}

static void nin_edit_conf_num(int n)
{
	DIR *dir;
	int i, ok;
	char *conf, *str;
	struct dirent *det;

	if (n <= 0) {
		fprintf(stderr, __err("error: config selection number cannot be <= 0.\n"));
		exit(EXIT_FAILURE);
	}

	i = 1;
	ok = 0;
	conf = nin_check_conf_dir();
	if (conf == NULL) {
		fprintf(stderr, __err("error: something went wrong, cannot continue...\n"));
		exit(EXIT_FAILURE);
	}
	dir = opendir(conf);
	if (dir == NULL) {
		perror("opendir()");
		free(conf);
		exit(EXIT_FAILURE);
	}

	while ((det = readdir(dir)) != NULL) {
		if (det->d_type == DT_REG) {
			str = strrchr(det->d_name, '.');
			if (str != NULL && (strcmp(str, ".conf") == 0)) {
				if (i == n) {
				        ok = 1;
					nin_edit_conf(trim_conf(det->d_name));
					break;
				}
				i++;
			}
		}
	}
	free(conf);
	closedir(dir);

	if (!ok) {
		fprintf(stderr,
			__err("error: config selection number %d was not found. "
			      "maximum config selection number is %d.\n"), n, i - 1);
		exit(EXIT_FAILURE);
	}
}

static void nin_del_conf(const char *name)
{
        char *conf;
	struct stat st;

	conf = nin_wrap_conf_path(name);
	if (conf == NULL)
		exit(EXIT_FAILURE);

	if (stat(conf, &st) == -1) {
	        fprintf(stderr, __err("error: couldn't find \"%s\" config.\n"), name);
		free(conf);
		exit(EXIT_FAILURE);
	}

	if (unlink(conf) == -1) {
		perror("unlink()");
		free(conf);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,
	        __ok("ok: deleted \"%s\"\n"), name);
	free(conf);
}

static void nin_del_conf_num(int n)
{
	DIR *dir;
	int i, ok;
	char *conf, *str;
	struct dirent *det;

	if (n <= 0) {
		fprintf(stderr, __err("error: config selection number cannot be <= 0.\n"));
		exit(EXIT_FAILURE);
	}

	i = 1;
	ok = 0;
	conf = nin_check_conf_dir();
	if (conf == NULL) {
		fprintf(stderr, __err("error: something went wrong, cannot continue...\n"));
		exit(EXIT_FAILURE);
	}
	dir = opendir(conf);
	if (dir == NULL) {
		perror("opendir()");
		free(conf);
		exit(EXIT_FAILURE);
	}

	while ((det = readdir(dir)) != NULL) {
		if (det->d_type == DT_REG) {
			str = strrchr(det->d_name, '.');
			if (str != NULL && (strcmp(str, ".conf") == 0)) {
			        if (i == n) {
					ok = 1;
					nin_del_conf(trim_conf(det->d_name));
					break;
				}
				i++;
			}
		}
	}
	free(conf);
	closedir(dir);

	if (!ok) {
		fprintf(stderr,
		        __err("error: config selection number %d was not found. "
			      "maximum config selection number is %d.\n"), n, i - 1);
		exit(EXIT_FAILURE);
	}
}

static void nin_set_root_run(char *path, char *name)
{
	char *p;
	size_t sz;

	sz = strlen(name) + (size_t)6;
	p = calloc(sz, sizeof(char));
	if (p == NULL) {
		perror("calloc()");
		exit(EXIT_FAILURE);
	}

	sprintf(p, "%s.conf", name);
        nin_exec_cmd(BOXPATH, "-P", path, "-O", p, NON_BLOCK);
	free(p);
}

static void nin_set_rom_run(char *path, char *name)
{
        char *conf;

	conf = nin_wrap_conf_path(name);
	if (conf == NULL)
		exit(EXIT_FAILURE);

        nin_exec_cmd(BOXPATH, "-R", path, "-O", conf, NON_BLOCK);
        free(conf);
}

static void nin_open_settings(const char *name)
{
        char *conf;

	conf = nin_wrap_conf_path(name);
	if (conf == NULL)
	        exit(EXIT_FAILURE);

	nin_exec_cmd(BOXPATH, "-C", conf, "-S", NULL, NON_BLOCK);
	fprintf(stdout, __ok("ok: opening settings for \"%s\"\n"), name);
	free(conf);
}

static void nin_view_conf(const char *name)
{
	FILE *fp;
	long lsz;
        char *conf;

	conf = nin_wrap_conf_path(name);
	if (conf == NULL)
		exit(EXIT_FAILURE);

	fp = fopen(conf, "r");
	if (fp == NULL) {
		fprintf(stderr, __err("error: %s config was not found.\n"), name);
	        free(conf);
		exit(EXIT_FAILURE);
	}

        free(conf);
	fseek(fp, (long)1, SEEK_END);
	lsz = ftell(fp);
	fseek(fp, (long)0, SEEK_SET);

	conf = calloc((size_t)(lsz + 1), sizeof(char));
	if (conf == NULL) {
		perror("calloc()");
		fclose(fp);
	        exit(EXIT_FAILURE);
	}

	fread(conf, (size_t)1, (size_t)lsz, fp);
	fprintf(stdout, "%s", conf);
	fclose(fp);
	free(conf);
}

static void nin_view_conf_num(int n)
{
	DIR *dir;
	int i, ok;
	char *conf, *str;
	struct dirent *det;

	if (n <= 0) {
		fprintf(stderr, __err("error: config selection number cannot be <= 0.\n"));
		exit(EXIT_FAILURE);
	}

	i = 1;
	ok = 0;
	conf = nin_check_conf_dir();
	if (conf == NULL) {
		fprintf(stderr, __err("error: something went wrong, cannot continue...\n"));
		exit(EXIT_FAILURE);
	}
	dir = opendir(conf);
	if (dir == NULL) {
		perror("opendir()");
		free(conf);
		exit(EXIT_FAILURE);
	}

	while ((det = readdir(dir)) != NULL) {
		if (det->d_type == DT_REG) {
			str = strrchr(det->d_name, '.');
			if (str != NULL && (strcmp(str, ".conf") == 0)) {
				if (i == n) {
					ok = 1;
					nin_view_conf(trim_conf(det->d_name));
					break;
				}
				i++;
			}
		}
	}
	free(conf);
	closedir(dir);

	if (!ok) {
		fprintf(stderr,
			__err("error: config selection number %d was not found. "
			      "maximum config selection number is %d.\n"), n, i - 1);
		exit(EXIT_FAILURE);
	}
}

static void usage(void)
{
	fputs(
	        "usage: ninbox [COMMAND] [ARGS] ...\n"
		"command: \n"
		"    --load-name [name]\n"
		"    --load-num [pos]\n"
		"      -> Load a machine config.\n\n"
		"    --edit-name [name]\n"
		"    --edit-num [pos]\n"
		"      -> Edit a machine config.\n\n"
		"    --del-name [name]\n"
		"    --del-num [pos]\n"
		"      -> Delete a machine config.\n\n"
		"    --view-name [name]\n"
		"    --view-num [pos]\n"
		"      -> Prints out (stdout) a machine config.\n\n"
		"    --list\n"
		"      -> List all available config.\n\n"
		"    --rom-path [path] [name]\n"
		"      -> Set a ROM directory for current session and load a config.\n\n"
		"    --root-path [path] [name]\n"
		"      -> Set a root directory for current session and load a config.\n\n"
		"    --settings [name]\n"
		"      -> Open settings for specific config.\n\n"

		"Info: Here, \"name\" specify the config name. (e.g. 'Windows 98')\n"
		"\"path\" specify the location and \"pos\" specify the position\n"
		"of the machine, which in here would be an integer value. (e.g. 2)\n"
		, stdout
		);
}

int main(int argc, char **argv)
{
	int opt, i;
	struct long_opts lopts = {
		.load_name_opt = 0, .load_num_opt  = 0,
		.edit_name_opt = 0, .edit_num_opt  = 0,
		.del_name_opt  = 0, .del_num_opt   = 0,
		.list_opt      = 0, .root_path_opt = 0,
		.rom_path_opt  = 0, .settings_opt  = 0,
		.view_name_opt = 0, .view_num_opt  = 0
	};
        struct option loptions[] = {
		{ "load-name", required_argument, 0, LOAD_NAME },
		{ "load-num",  required_argument, 0, LOAD_NUM },
		{ "edit-name", required_argument, 0, EDIT_NAME },
		{ "edit-num",  required_argument, 0, EDIT_NUM },
		{ "del-name",  required_argument, 0, DEL_NAME },
		{ "del-num",   required_argument, 0, DEL_NUM },
		{ "list",      no_argument,       0, LIST },
		{ "root-path", required_argument, 0, ROOT_PATH },
		{ "rom-path",  required_argument, 0, ROM_PATH },
		{ "settings",  required_argument, 0, SETTINGS },
		{ "view-name", required_argument, 0, VIEW_NAME },
		{ "view-num",  required_argument, 0, VIEW_NUM },
		{ "help",      no_argument,       0, USAGE },
		{ 0,           0,                 0, 0 }
	};

	if (argc < 2 || argv[1][0] != '-' ||
	    (argv[1][0] == '-' && argv[1][1] == '\0') ||
	    (argv[1][0] == '-' && argv[1][1] == '-' && argv[1][2] == '\0')) {
		usage();
	        goto clean_exit;
	}

	while ((opt = getopt_long(argc, argv, "", loptions, NULL)) != -1) {
		switch (opt) {
		case LOAD_NAME:
		        lopts.load_name_opt = 1;
		        break;

		case LOAD_NUM:
			lopts.load_num_opt = 1;
		        break;

		case EDIT_NAME:
			lopts.edit_name_opt = 1;
			break;

		case EDIT_NUM:
			lopts.edit_num_opt = 1;
			break;

		case DEL_NAME:
			lopts.del_name_opt = 1;
			break;

		case DEL_NUM:
			lopts.del_num_opt = 1;
			break;

		case LIST:
			lopts.list_opt = 1;
			break;

		case ROOT_PATH:
			lopts.root_path_opt = 1;
			break;

		case ROM_PATH:
			lopts.rom_path_opt = 1;
			break;

		case SETTINGS:
			lopts.settings_opt = 1;
			break;

		case VIEW_NAME:
			lopts.view_name_opt = 1;
			break;

		case VIEW_NUM:
			lopts.view_num_opt = 1;
			break;

		case USAGE:
		        usage();
			goto clean_exit;

		default:
			exit(EXIT_FAILURE);
		}
	}

	if (lopts.load_name_opt) {
		args_iterate_over(nin_load_conf(argv[i]));
		goto clean_exit;
	}

	if (lopts.load_num_opt) {
	        args_iterate_over(nin_load_conf_num(atoi(argv[i])));
	        goto clean_exit;
	}

	if (lopts.edit_name_opt) {
		args_iterate_over(nin_edit_conf(argv[i]));
	        goto clean_exit;
	}

	if (lopts.edit_num_opt) {
		args_iterate_over(nin_edit_conf_num(atoi(argv[i])));
	        goto clean_exit;
	}

	if (lopts.del_name_opt) {
		args_iterate_over(nin_del_conf(argv[i]));
	        goto clean_exit;
	}

	if (lopts.del_num_opt) {
		args_iterate_over(nin_del_conf_num(atoi(argv[i])));
	        goto clean_exit;
	}

	if (lopts.list_opt) {
		for (i = 2; i < argc; i++)
			single_check_multiple_args(i);
		nin_list_conf();
		goto clean_exit;
	}

        if (lopts.root_path_opt) {
		for (i = 2; i < argc; i++)
			single_check_multiple_args(i);

	        if (argc < 4) {
		        fputs(__err("error: config name is a required argument.\n"), stderr);
			exit(EXIT_FAILURE);
		}
	        nin_set_root_run(argv[2], argv[3]);
		goto clean_exit;
	}

	if (lopts.rom_path_opt) {
		for (i = 2; i < argc; i++)
			single_check_multiple_args(i);

		if (argc < 4) {
		        fputs(__err("error: config name is a required argument.\n"), stderr);
			exit(EXIT_FAILURE);
		}
		nin_set_rom_run(argv[2], argv[3]);
		goto clean_exit;
	}

	if (lopts.settings_opt) {
		for (i = 2; i < argc; i++)
			single_check_multiple_args(i);
		nin_open_settings(argv[2]);
		goto clean_exit;
	}

	if (lopts.view_name_opt) {
	        args_iterate_over(nin_view_conf(argv[i]));
		goto clean_exit;
	}

	if (lopts.view_num_opt) {
		args_iterate_over(nin_view_conf_num(atoi(argv[i])));
		goto clean_exit;
	}

clean_exit:
	exit(EXIT_SUCCESS);
}
