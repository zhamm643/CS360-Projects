// CS360 -- Lab 2 -- Buffering
// Professor: Jian Huang

// l2p2
// Zachary Hamm
// netid: zhamm
// 9-15-21


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "jrb.h"
#include "jval.h"
#include "dllist.h"
typedef struct ip {
    unsigned char addr[4];
    JRB names_tree;

} ip;

int main() {
    unsigned char name_buf[4];
    unsigned char c[2] = {};
    char machine_name[1000];
	int fd, sz;
		int h = 0;
	  // using open() to read file as directed in lab writeup
	fd = open("converted", O_RDONLY);

    JRB hosts, tmp;
    JRB tmp2, tmp3, tmp4;
    Dllist tmp_list;
    Dllist tmp_list2;
    ip *p;

    hosts = make_jrb();

    
    while (1) {
		
		// allocates new memory each time a new ip is being ran
        p = malloc(sizeof(ip));
        p->names_tree = make_jrb();
			
		  // using read() to store first 4 bytes into p->addr
		  if(read(fd, p->addr, 4) != 4) {
			free(p);
			break;
		  }
		  
		  // using read() to store next 4 bytes into name_buf
		  if(read(fd, name_buf, 4) != 4) {
			free(p);
			break;
		  }

		  // converting name_buf into format needed e.g (0 0 0 1) - > (1)
          u_int32_t num_names = (name_buf[0] << 24) + (name_buf[1] << 16) + (name_buf[2] << 8) +
			  name_buf[3];

        
	    // need to run this loop the amount of names each machine is known by
        for (int i = 0; i < num_names; i++) {
            int holder = 0;
            machine_name[0] = '\0';

			 while(read(fd, c, 1) == 1) {
			 // using read to read 1 byte at a time into c
				
				// concats each char to machine_name
                strncat(machine_name, c, 1);
				
				// this creates local name from absolute name
                if ((c[0] == '.') && (holder == 0)) {
                    machine_name[strlen(machine_name) - 1] = '\0';
                    char *tmp_name = strdup(machine_name);
                    jrb_insert_str(p->names_tree, tmp_name, new_jval_v(tmp_name));
                    holder++;
                    char temp = '.';
                    machine_name[strlen(machine_name)] = temp;
                    machine_name[strlen(machine_name) + 1] = '\0';
                }
				// when this runs, I know I have the full name of the machine
                if (c[0] == '\0') {
                    char *tmp_name = strdup(machine_name);

					// insert name into jrb tree inside my struct.
                    jrb_insert_str(p->names_tree, tmp_name, new_jval_v(tmp_name));
                    break;
                } // end of if

             }  // end of while

        } // end of for

		// traverse through p->names_tree
        jrb_traverse(tmp2, p->names_tree) {
		
			// if names exists in hosts tree, append it to the list
            if (jrb_find_str(hosts, tmp2->val.v)) {
                tmp3 = jrb_find_str(hosts, tmp2->val.v);
                dll_append(tmp3->val.v, new_jval_v(p));
			// if name doesn't exist, create list, append name, insert list keyed on name
            } else {
                tmp_list = new_dllist();
                dll_append(tmp_list, new_jval_v(p));
                jrb_insert_str(hosts, tmp2->val.v, new_jval_v(tmp_list));
            }
        }

    } // end of while

    char str[100];
    printf("Hosts all read in\n");
    while (!feof(stdin)) {
        printf("\nEnter host name: ");
		// this code handles user input of just pressing enter on keyboard
        if(fgets(str, 100, stdin) == NULL)
			break;
		if(strcmp(str, "\n") == 0)
			continue;
		for(int i = 0; i < strlen(str); i++) {
			if(str[i] == '\n') {
				str[i] = '\0'; break;
			}
		}
		
		
        if (tmp = jrb_find_str(hosts, str)) {
				    
				tmp_list = tmp->val.v;
				// traversing my names_list so i can access the correct ip for each name
				dll_traverse(tmp_list2, tmp_list) {
				 p = tmp_list2->val.v;
                  for (int i = 0; i < 4; i++) {
                    printf("%d", p->addr[i]);
                    if (i < 3)
                        printf(".");
                  }
						printf(": ");
				// traversing names_tree to print out correct name or names
				  jrb_traverse(tmp3, p->names_tree) { 
					printf("%s ", jval_s(tmp3->val));
				
				  }  
					printf("\n");
				 }
            
        } else {
            printf("no key %s\n", str);
        }
    } // end of while

} // end of main
