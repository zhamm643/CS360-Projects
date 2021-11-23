/* 
 
   CS360 Lab 3 - "Fakemake"
   Author - Zachary Hamm
   Professor - Jian Huang
   Date: September 16th, 2021

*/

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fields.h"
#include "dllist.h"
#include "jrb.h"
#include "jval.h"

typedef struct List {

	Dllist c_list;
	Dllist h_list;
	Dllist lib_list;
	Dllist f_list;
	Dllist o_list;

} List;

void remake(List *list_ptr, char *file_name) {
	Dllist tmp;
	char *final = malloc(1000);
	strcpy(final, "gcc -c ");
	
	// add flags to string from my flags list
	dll_traverse(tmp, list_ptr->f_list) {

		strcat(final, tmp->val.s);
		strcat(final, " ");
	}

	strcat(final, file_name);
	printf("%s\n", final);

	if(system(final) != 0) {
		fprintf(stderr, "Command failed.  Exiting\n");
		exit(1);
	}
	free(final);
}


/* The function process_h traverses through
 * the 'H' Dllist and calls stat on each file.
 * It flags an error if the file doesn't exist.
 * If the file does exist, it returns the max
 * value to the main program. 
 */ 

time_t process_h (List *list_ptr, Dllist file_list) {
	Dllist tmp;
	time_t max;
	int found;

	struct stat buf;
	// I set the max to the first file in list, then check if any are bigger
	found = stat(dll_first(file_list)->val.s, &buf);
	max = buf.st_mtime;

	dll_traverse(tmp, file_list) {
		found = stat(tmp->val.s, &buf);
		if(found == -1) {
			fprintf(stderr, "Header file '%s' not found\n", tmp->val.s);
				exit(-1);
		}
		if(buf.st_mtime > max)
			max = buf.st_mtime;
	}
	// this will return the biggest value
	return max;
}


/* The function process_c traverse through
 * the 'C' Dllist and calls stat on each file.
 * It looks for the .o file, if it doesn't exist
 * or is less recent then I remake the .o file.
 */

void process_c (List *list_ptr, int h_time, char *exec) {
	Dllist tmp;
	int incr = 0;
	char* o_ptr;
	struct stat buf;
	struct stat buf2;

	dll_traverse(tmp, list_ptr->c_list) {

		if(stat(tmp->val.s, &buf) == 0) {
			
			o_ptr = strdup(tmp->val.s);


			o_ptr[strlen(o_ptr)-1] = 'o';

			// adds .o file to o list
			dll_append(list_ptr->o_list, new_jval_s(strdup(o_ptr)));
	
			// looks if .o file exists
			if(stat(o_ptr, &buf2) == 0) {
	
			 // checks if file is less recent than c file.
				if(buf2.st_mtime < h_time) {
					remake(list_ptr, tmp->val.s);
				}
			 // checks if file is less recent than the maximum st_mtime of the header files.
				else if(buf2.st_mtime < buf.st_mtime) {
					remake(list_ptr, tmp->val.s);
				}
				else {
				// file doesn't need to be remade
				}
		
			}
			// no o file exists, need to make it
			else {
				remake(list_ptr, tmp->val.s);
			}	       

	  }
	  else {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", tmp->val.s);
			exit(1);
	   }
	  

	}
	// no memory leaks I hope :)
	free(o_ptr);
	

	if(stat(exec, &buf) == 0) {
		int o_time = process_h(list_ptr, list_ptr->o_list);

		if(o_time > buf.st_mtime)
			incr = 1;
	}
	else {
		incr = 1;
	}
	if(incr == 1) {
		char *final = malloc(1000);

		strcat(final, "gcc -o ");
		strcat(final, exec);
		strcat(final, " ");
		// Example: gcc -o f 
		
		// this traverses the flags list and adds flag
		dll_traverse(tmp, list_ptr->f_list) {
			strcat(final, tmp->val.s);
			strcat(final, " ");
		}
		// Example: gcc -o f -g

		// this traverse o list and adds o files
		dll_traverse(tmp, list_ptr->o_list) {
			strcat(final, tmp->val.s);
			strcat(final, " ");
		}
		// Example: gcc - o f -g f.o f1.o f2.o

		// this adds library list at end of gcc command
		dll_traverse(tmp, list_ptr->lib_list) {
			strcat(final, tmp->val.s);
			strcat(final, " ");
		}

		final[strlen(final) - 1] = '\0';
		printf("%s\n", final);

		if(system(final) != 0) {
			fprintf(stderr, "Command failed.  Fakemake exiting\n");
			exit(1);
		}
		// free up the string
		free(final);
	} 
	else {
		printf("%s up to date\n", exec);
	}


} // end of process_c

/* The function free_list traverses
 * through a list and frees everything
 */

void free_list(Dllist list) {
	Dllist tmp;

	dll_traverse(tmp, list) {
		free(tmp->val.s);
	}
	free_dllist(list);
}

int main(int argc, char *argv[]) {

	// All error checking done how
	// solution executable has it

	// all my variables
	char *temp_file;
	char *exec_name = NULL;
	IS is;
	List *list_ptr;
	int h_time = 0;	

	list_ptr = malloc(sizeof(List));
	// creating all of my lists
	list_ptr->c_list = new_dllist();
	list_ptr->h_list = new_dllist();
	list_ptr->lib_list = new_dllist();
	list_ptr->f_list = new_dllist();
	list_ptr->o_list = new_dllist();

	// fakemake [ description-file ]
	// if no description file is specified
	// assume that the file is fmakefile

	if(argc == 1) {
		temp_file = "fmakefile";
		is = new_inputstruct(temp_file);
		if(is == NULL) {
			fprintf(stderr, argv[1]);
			exit(1);
		}
	}
	if(argc == 2) {
		temp_file = argv[1];
		is = new_inputstruct(temp_file);
		if(is == NULL) {
			fprintf(stderr, "file: %s does not exist.\n", argv[1] );
			exit(1);
		}
	}

	while(get_line(is) >= 0) {
		if(is->NF != 0) {

			if(*is->fields[0] == 'E') {
				if(exec_name != NULL) {
				  fprintf(stderr, "fmakefile (%i) cannot have more than one E line\n", is->line);
				  exit(1);
				}
				else {
					exec_name = strdup(is->fields[1]);
				}
			}
			// Reads every line and appends the right information into the according lists
			else if (strcmp(is->fields[0], "C") == 0) {
				for(int i = 1; i < is->NF; i++)
					dll_append(list_ptr->c_list, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(is->fields[0], "H") == 0) {
					for(int i = 1; i < is->NF; i++)
						dll_append(list_ptr->h_list, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(is->fields[0], "L") == 0) {
					for(int i = 1; i < is->NF; i++)
						dll_append(list_ptr->lib_list, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(is->fields[0], "F") == 0) {
					for(int i = 1; i < is->NF; i++)
						dll_append(list_ptr->f_list, new_jval_s(strdup(is->fields[i])));
			}
			else {
				fprintf(stderr, "fakemake (%d): Lines must start with C, H, L, F or E\n", is->line);
				exit(1);
			}
		  

		}

	}	// end of while

		if(exec_name == NULL) {
			fprintf(stderr, "No executable specified\n");
			exit(1);
		}


	if(list_ptr->h_list != NULL) {
		h_time = process_h(list_ptr, list_ptr->h_list);
	}
		process_c(list_ptr, h_time, exec_name);
	
		// free all my lists, close file
		free_list(list_ptr->c_list);
		free_list(list_ptr->h_list);
		free_list(list_ptr->o_list);
		free_list(list_ptr->lib_list);
		free_list(list_ptr->f_list);

		free(exec_name);
		free(list_ptr);

		jettison_inputstruct(is);

	exit(0);

} // end of main
