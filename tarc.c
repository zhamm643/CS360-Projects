// Zach Hamm
// Systems Programming CS360
// Lab 4 Jtar - Tarc
// Oct 8th, 2021
// 

#include "fields.h"
#include "jrb.h"
#include "dllist.h"
#include "jval.h"

#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

void print_tar(bool unique_inode, char *path, char *relative, struct stat *buf, JRB inodes) {
	unsigned int length, mode;
	unsigned long inode, time, file_size;
	FILE *file;
	char byte;

	length = strlen(relative); // size of file's name - 4 bytes
	inode = buf->st_ino; // the inode of file - 8 bytes
	mode = buf->st_mode; // the mode of file - 4 bytes
	time = buf->st_mtime; // last modified time - 8 bytes
	file_size = buf->st_size; // size of file - 8 bytes

	// Printing
	
	fwrite(&length, 4, 1, stdout);
	printf("%s", relative); // name of file
	fwrite(&inode, 8, 1, stdout);
	
	// First time we have seen a file with that inode
	if (unique_inode) {	

		// printing mode and last modified time, in correct format
		fwrite(&mode, 4, 1, stdout);
		fwrite(&time, 8, 1, stdout);
	}

	// If the file is a file and not a directory

	if(!S_ISDIR(buf->st_mode) && unique_inode) {

		// printing file size, in correct format
		fwrite(&file_size, 8, 1, stdout);

		// printing the rest of file
		file = fopen(path, "r");
		byte = fgetc(file);
		while(1) {
			if(feof(file)) {
				break;
			}
			printf("%c", byte);
			byte = fgetc(file);
		}
		fclose(file);
	}
}

void make_tar(char *absolute, JRB inodes, char *filename) {


	DIR *directory;
	char *path, *relative;

	struct stat buf;
	struct dirent *dirbuf;

	bool unique_inode = false;

	int info;

	Dllist tmp_dir, dir_list;
	dir_list = new_dllist();

	directory = opendir(filename);

//	path = malloc(strlen(filename));
	path = malloc(strlen(filename) + 258);
	
	while((dirbuf=readdir(directory)) != NULL) {


		sprintf(path, "%s/%s", filename, dirbuf->d_name);
		info = lstat(path, &buf);

		if(info == -1)
			fprintf(stderr, "Could not stat %s\n", path);

		if((strcmp(dirbuf->d_name, ".") != 0) && (strcmp(dirbuf->d_name, "..") != 0)) {
	
				// check if inode is already in tree
				if(jrb_find_int(inodes, buf.st_ino) == NULL) {
					
					jrb_insert_int(inodes, buf.st_ino, new_jval_i(1));
					unique_inode = true;
				}

				if(absolute != NULL) {
					int i = strlen(absolute);
					relative = &path[i];
				} else {
					relative = path;
				}

				print_tar(unique_inode, path, relative, &buf, inodes);
				unique_inode = false;
		}

		// check if file is directory
		if (S_ISDIR(buf.st_mode)) {
			
		   if((strcmp(dirbuf->d_name, "..") != 0) && (strcmp(dirbuf->d_name, ".") != 0)) {

			   dll_append(dir_list, new_jval_s(strdup(path)));
		   }
	
		}

	} // end of while

	closedir(directory);

	dll_traverse(tmp_dir, dir_list) {
		make_tar(absolute, inodes, tmp_dir->val.s);
	}
	free(path);
	free_dllist(dir_list);

}// end of function

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "incorrect usage");
		exit(1);
	}
	struct stat buf;
	char *absolute, *relative, *tmp;
	DIR *directory = opendir(argv[1]);
	char *path = argv[1];
	JRB inodes;
	inodes = make_jrb();

	if(argv[1][0] == '/') {
		tmp = strdup(argv[1]);
		absolute = tmp;
		relative = strrchr(tmp, '/');
		relative++;
		relative[0] = '\0';

	} else {
		absolute = NULL;
	}

	if (directory == NULL) {
		fprintf(stderr, "Directory does not exist");
		exit(1);
	}

	int info = lstat(path, &buf);

	if(absolute != NULL) {
		int temp = strlen(absolute);
		relative = &path[temp];
	} else {
		relative = path;
	}

	if(info == 0) {
		print_tar(true, path, relative, &buf, inodes);
		closedir(directory);
	}

	make_tar(absolute, inodes, argv[1]);


	return 0;

} // end of main




