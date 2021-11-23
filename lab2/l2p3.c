// CS360 -- Lab 2 -- Buffering
// Professor: Jian Huang

// l2p3
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
	// massive buffer to store all of file "converted"
	char buffer[350000];
	int tracker = 0;
	int count = 0;
	int fd, sz;
	 // using open() to read file converted
     fd = open("converted", O_RDONLY);

	 // reading the entire file of converted into buffer
	 sz = read(fd, buffer, 350000);

    JRB hosts, tmp;
    JRB tmp2, tmp3, tmp4;
    Dllist tmp_list;
    Dllist tmp_list2;
    ip *p;

    hosts = make_jrb();

    while (1) {
		// tracker is used to track where in the buffer I currently need to read from
		// when tracker reaches sz (the size of buffer) I need to break out of the loop
		if(tracker == sz)
			break;
		// allocating new memory for each new ip struct
         p = malloc(sizeof(ip));
         p->names_tree = make_jrb();

		  // copying 4 bytes into p->addr
		  memcpy(p->addr, buffer+tracker, 4);
		  // incrementing tracker by 4 so I know what I have read in the file so far
		  tracker+=4;
		
		  // copying 4 bytes into name_buf
		  memcpy(name_buf, buffer+tracker, 4);
		  tracker+=4;

		  // using bit shifting to convert name_buf into an int
          u_int32_t num_names = (name_buf[0] << 24) + (name_buf[1] << 16) + (name_buf[2] << 8) +
			name_buf[3];
			
			 // run this loop the amount of names each machine is known by
			 for (int i = 0; i < num_names; i++) {
			  int holder = 0;
			  char temp_buf[100];
              machine_name[0] = '\0';
			   while(1) {
	  
				// copy 1 byte at the time into c
				memcpy(c, buffer+tracker, 1);
				// increment tracker each time a byte is read
				tracker+=1;

				// create string from those bytes
                strncat(machine_name, c, 1);
				
				// this code creates local name from absolute name
                if ((c[0] == '.') && (holder == 0)) {
                    machine_name[strlen(machine_name) - 1] = '\0';
                    char *tmp_name = strdup(machine_name);
                    jrb_insert_str(p->names_tree, tmp_name, new_jval_v(tmp_name));
                    holder++;
                    char temp = '.';
                    machine_name[strlen(machine_name)] = temp;
                    machine_name[strlen(machine_name) + 1] = '\0';
                }
				
				// when this runs I know I have the full machine name
                if (c[0] == '\0') {
                    char *tmp_name = strdup(machine_name);
					
					// insert name into jrb tree inside my struct
                    jrb_insert_str(p->names_tree, tmp_name, new_jval_v(tmp_name));
                    break;
                } // end of if
				
            } // end of while

        } // end of for

        jrb_traverse(tmp2, p->names_tree) {
			// if name exists in hosts tree, find it and append it to list
            if (jrb_find_str(hosts, tmp2->val.v)) {
                tmp3 = jrb_find_str(hosts, tmp2->val.v);
                dll_append(tmp3->val.v, new_jval_v(p));

            } else {
				// if name doesn't exist, create list, append name, insert list 
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
				// traversing my names_list so I can access the correct ip for each name
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
