// Zachary Hamm
// Lab 4 Jtar - Tarx
// Oct 8th, 2021

#include "fields.h"
#include "jrb.h"
#include "dllist.h"
#include "jval.h"

#include <fcntl.h>
#include <utime.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

typedef struct {
	char *file_name;
	long inode, mod_time, file_size;
	char *data;
	char *link_name;
	int mode;
} File;

int main(int argc, char **argv) {
	
	// Variables
	int name_size;
	bool intree = false; 
	struct utimbuf time_buf;

	File *f;
	File *f_ptr;
	Dllist files, tmp, tmp2;
	JRB inodes;

	// Open the file for reading and make structures
	inodes = make_jrb();
	files = new_dllist();
	int fd;
	int expected_value;


	while ((expected_value = fread(&name_size,1,4,stdin))) {
		if (expected_value != 4)
			fprintf(stderr, "Bad tarc file at byte %ld.\n", ftell(stdin));

		f = malloc(sizeof(File));
		intree = false;

		// Read in the name of the file
		f->file_name = malloc(name_size+1);

		expected_value = fread(f->file_name, 1, name_size, stdin);
			if (expected_value != name_size) {
			 fprintf(stderr, "Bad tarc file at byte %ld. "
					 " File name size is %i, but bytes read = %i.\n", ftell(stdin), 
					 name_size, expected_value);
			  exit(1);
			}

		expected_value = fread(&f->inode, 1, 8, stdin);
			if (expected_value != 8) {
				fprintf(stderr, "Bad tarc file for %s.  Couldn't read inode\n", f->file_name);
				exit(1);
			}
		

		// If the inode is unique, store the bytes in a tree
		if (jrb_find_int(inodes, f->inode) == NULL) {

			jrb_insert_int(inodes, f->inode, new_jval_v(f));
			intree = true;

		}
		 if(intree) {
		
			// reading mode and last modified time
			expected_value = fread(&f->mode, 1, 4, stdin);
			if(expected_value != 4) {
				fprintf(stderr, "Bad tarc file for %s.  Couldn't read mode\n", f->file_name);
				exit(1);
			}
			fread(&f->mod_time, 8, 1, stdin);
		
		 } 
			// If it's a file and not directory
		 if (!S_ISDIR(f->mode) && intree) {
				
			// read in the file file_size
			expected_value = fread(&f->file_size, 1, 8, stdin);
			if(expected_value != 8) {
				fprintf(stderr, "Bad tarc file for %s.  Couldn't read size\n", f->file_name);
				exit(1);
			}

			// read in the bytes in the file
			f->data = malloc(f->file_size);
			expected_value = fread(f->data, 1, f->file_size, stdin);
			if(expected_value != f->file_size) {
				fprintf(stderr, "Bad tarc file for %s. "
						" Trying to read %ld bytes of the file, and got EOF\n",
						f->file_name, f->file_size);
						exit(1);
			}


		 }
			 

		// if inode is not unique
		else {
			// 
			f_ptr = jrb_find_int(inodes, f->inode)->val.v;
			f->link_name = malloc(sizeof(f_ptr->file_name));

			// link_name name, mode, and modified time
			f->link_name = strdup(f_ptr->file_name);
			f->mode = f_ptr->mode;
			f->mod_time = f_ptr->mod_time;

			// If it is a file, also read file size and rest of file
			if (!S_ISDIR(f->mode)) {
				f->file_size = f_ptr->file_size;
				f->data = strdup(f_ptr->data);
			}
			
		} 

		// append the file to a dllist
		dll_append(files, new_jval_v(f));
	}

	// traverse through list of files
	dll_traverse(tmp, files) {
		// set f to the pointer connected to current file
		f = tmp->val.v;
		if (S_ISDIR(f->mode)) {
			mkdir(f->file_name, 0777);
		}
		else {

			// If inode isn't unique, create a hard link
			if (f->link_name != NULL) {
				link(f->link_name, f->file_name);
			}

			// inode is new, create new file to store information
			if (f->link_name == NULL) {
				fd = open(f->file_name, O_WRONLY | O_CREAT);
				write(fd, f->data, f->file_size);
				
				close(fd);
	
			}
		}
	}
	
	//
	dll_traverse(tmp2, files) {
		f = tmp2->val.v;

		if(chmod(f->file_name, f->mode) != 0)
			fprintf(stderr, "chmod() error");
		
		time_buf.modtime = f->mod_time;
		utime(f->file_name, &time_buf);

	} 

	// Free list and tree
	jrb_free_tree(inodes);
	free_dllist(files);

	return 0;
} // end of main
