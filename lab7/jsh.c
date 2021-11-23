
// Zach Hamm
// November 21st, 2021
// CS360 - Jian Huang
// JSH lab (shell)
// Parts 1, 2, 3

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "fields.h"
#include "jval.h"
#include "jrb.h"

int main(int argc, char **argv, char **envp) {

	char *prompt;
	char ***arg_arr;

	int status, i, j, pid, fd, fd2, failed, last_pipe;
	int pipefd[2];

	int num_pipes = 0;
	int final_pipe = 0;

	bool input_arrow = false;
	bool output_arrow = false;
	bool pipein = false;
	bool pipeout = false;
	bool waiting = false;

	JRB tree, tmp;
	tree = make_jrb();

	IS is;
	is = new_inputstruct(NULL);
	
	 if(argc > 2) {
		fprintf(stderr,"Incorrect number of parameters.");
			exit(0);
		}

	 if(argv[1] == NULL) {

			printf("jsh3: ");
			prompt = "jsh3:";
	 }
	 else if (strcmp(argv[1], "-") != 0) {

			printf("%s: ", argv[1]);
			prompt = argv[1];
		}
		// get entire line
		while(get_line(is) >= 0) {
            num_pipes = 0;
            waiting = false; 

			if(argv[1] == NULL)
				printf("%s", prompt);

			// check for <, >, or >>
			for(i = 0; i < is->NF; i++) {
				if(strcmp(is->fields[i], "<") == 0) {
					input_arrow = true;
					output_arrow = true;
					break;
				}
				else if(strcmp(is->fields[i], ">") == 0 || strcmp(is->fields[i], ">>") == 0) {
					output_arrow = true;
					input_arrow = true;
					break;
				}
			}
			// check for 'exit' 
			if(is-> NF == 1) {
				if(strcmp(is->fields[0], "exit") == 0) {
				exit(0);
				}
			}
			
			is->fields[is->NF] = NULL;

			num_pipes = 0;
			if(is->NF > 0) {
				// check for & for waiting
				if (strcmp(is->fields[is->NF-1], "&") == 0) {
					is->fields[is->NF-1] = NULL;
					is->NF = (is->NF-1);
					waiting = true;
				}


			// check and count number of pipes
				for(i = 0; i < is->NF; i++) {
				if(strcmp(is->fields[i],"|") == 0) 
					num_pipes++;
				}

			//malloc argument array
			arg_arr = (char ***) malloc(sizeof(char **) * (num_pipes+1));
			
			
			num_pipes = 0;
			arg_arr[0] = (char **)is->fields;

			for(i = 0; i < is->NF; i++) {
				if(strcmp(is->fields[i],"|") == 0) {
					arg_arr[++num_pipes] = (is->fields + i + 1);
					final_pipe = num_pipes;
					is->fields[i] = NULL;
				}
			}
	
				
			for(i = 0; i <= num_pipes; i++) {

				// this handles my pipes between processes, first>mid>final
				if(num_pipes != 0) {
			    	if(i == 0) {
						pipe(pipefd);
						fd2 = pipefd[1];
						pipeout = true;
					}
					else if ((i > 0) & (i != final_pipe)) {
						close(pipefd[1]);
						fd = pipefd[0];
						last_pipe = pipefd[0];
						pipe(pipefd);
						fd2 = pipefd[1];
						pipeout = true;
						pipein = true;
					}
					else if (i == final_pipe) {
						fd = pipefd[0];
						close(pipefd[1]);
						pipein = true;
					}
				 
				}
				

				//Fork
				pid = fork();
				if(pid == 0) {
					if((i == 0) | (i == final_pipe)) {
						// this for loop, loops through argument array, checking for all arguments
						for(j = 1; arg_arr[i][j] != NULL; j++) {
							
							if(strcmp(arg_arr[i][j],"<") == 0) {	
								if(input_arrow) {
									fd = open(arg_arr[i][j+1],O_RDONLY);
									if(fd < 0) { 
										perror(arg_arr[i][j+1]);
										exit(1);
									}
									pipein = true;
									arg_arr[i][j] = NULL;
								}
							}
						 

							else if((strcmp(arg_arr[i][j],">") == 0) || (strcmp(arg_arr[i][j], ">>") == 0)) { //Output redirection
								if(output_arrow) {

									if(strcmp(arg_arr[i][j],">") == 0) {
										fd2 = open(arg_arr[i][j+1], O_TRUNC | O_WRONLY | O_CREAT, 0644);
									}
									if(strcmp(arg_arr[i][j],">>") == 0) {
										fd2 = open(arg_arr[i][j+1], O_APPEND | O_WRONLY | O_CREAT, 0644);
									}

									if(fd2 < 0) {
										perror(arg_arr[i][j+1]);
										exit(1);
									}
									pipeout = true;
									arg_arr[i][j] = NULL;
							   }
						   } 
					
					  } // end of for
				   } 

					//Dupes the input pipe
					if(pipein) {
                        
                        if(dup2(fd,0) == 0) {
                            close(fd);
                            pipein = false;
                        } else {
                            perror("Can't dup input");
                        }
					}

					//Dupes the output pipe
					if(pipeout) {

                        if(dup2(fd2,1) == 1) {
                            close(fd2);
                            pipeout = false;
                        } else {
                            perror("Can't dup output");
                        }
					}


					// execvp error checking
					failed = execvp(arg_arr[i][0],arg_arr[i]);	
					if(failed == -1) {
						perror(arg_arr[i][0]);
						exit(0);
					}
				//	if pid != 0 run this else
				} else { 

					if(num_pipes > 0) {

						if(i < final_pipe) {
							close(fd2);
						}
						if((i > 0) & (i < final_pipe)) {
							close(last_pipe);
						}
						if(i == final_pipe) {
							close(fd);
						}
					}
					
					//reset pipe bools to default
					pipein = false;
					pipeout = false;

					//make JRB tree if program is told to wait with &
					if(!waiting) {
						jrb_insert_int(tree,pid,new_jval_v(NULL));
					}
				
				} //end if
			} //end of for loop

			if(!waiting) {
				while(jrb_empty(tree) == 0) {
					if((tmp = jrb_find_int(tree,wait(&status))) != NULL) {
						jrb_delete_node(tmp);
					}
				}
			}

		}
		// free argument array 
		free(arg_arr);
	}
}
