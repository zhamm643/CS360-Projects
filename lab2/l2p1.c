// CS360 -- Lab 2 -- Buffering
// Professor: Jian Huang

// l2p1 
// Zachary Hamm
// netid: zhamm
// 9-15-21

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jrb.h"
#include "jval.h"
#include "dllist.h"

typedef struct ip {
    unsigned char addr[4];
    JRB names_tree;

} ip;

int main() {
    unsigned char name_buf[4];
    FILE *fileptr;
    int filelength;
    unsigned char c;
    char machine_name[1000];

    fileptr = fopen("converted", "rb");

    filelength = ftell(fileptr);

    JRB hosts, tmp;
    JRB tmp2, tmp3, tmp4;
    Dllist tmp_list;
    Dllist tmp_list2;
    ip *p;

    hosts = make_jrb();

    while (1) {
        if (feof(fileptr)) { 
            break;
			// break out of this loop after file has been read
        }
		// everytime I get back here, i need a new ip struct.
        p = malloc(sizeof(ip)); 
        p->names_tree = make_jrb();

        fread(p->addr, 4, 1, fileptr);  
		// read 4 bytes into addr for ip

        fread(name_buf, 4, 1, fileptr);
		// read in next 4 bytes for # of names
        u_int32_t num_names = (name_buf[0] << 24) + (name_buf[1] << 16) + (name_buf[2] << 8) +
            name_buf[3];
		// used bit shifting to change into correct format

		// need to run this loop the amount of names each machine is known by
        for (int i = 0; i < num_names; i++) {
		    int holder = 0;
            machine_name[0] = '\0';
             while (1) {
				// if end of file is reached, break out of this loop
                if (feof(fileptr))
                    break;
				
				// reads in 1 byte at a time to c
                c = fgetc(fileptr);
			
				// concats that char to machine_name
                strncat(machine_name, &c, 1);
				
				// this creates local name from absolute name
                if ((c == '.') && (holder == 0)) {
                    machine_name[strlen(machine_name) - 1] = '\0';
                    char *tmp_name = strdup(machine_name);
                    jrb_insert_str(p->names_tree, tmp_name, new_jval_v(tmp_name));
                    holder++;
                    char temp = '.';
                    machine_name[strlen(machine_name)] = temp;
                    machine_name[strlen(machine_name) + 1] = '\0';
                }
				
				// when this statement runs, I know I have the full name of the machine
                if (c == '\0') {
                    char *tmp_name = strdup(machine_name);
					
					// insert the name into my jrb tree inside my ip struct,
                    jrb_insert_str(p->names_tree, tmp_name, new_jval_v(tmp_name));
                    break;
                } // end of if

             } // end of while

        } // end of for
		
		// traverses through p->names_tree
        jrb_traverse(tmp2, p->names_tree) {
		
			// if name exists in hosts tree, append it to the list
            if (jrb_find_str(hosts, tmp2->val.v)) {
                tmp3 = jrb_find_str(hosts, tmp2->val.v);
                dll_append(tmp3->val.v, new_jval_v(p));

            } else {
				// if name doesn't exist, create list, append name, insert list keyed on name
                tmp_list = new_dllist();
                dll_append(tmp_list, new_jval_v(p));
                jrb_insert_str(hosts, tmp2->val.v, new_jval_v(tmp_list));
            }
        }

    } // end of while

    fclose(fileptr);

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
				// traversing my names list
				dll_traverse(tmp_list2, tmp_list) {

				 p = tmp_list2->val.v;
                  for (int i = 0; i < 4; i++) {
                    printf("%d", p->addr[i]);
                    if (i < 3)
                        printf(".");
                  }
						printf(": ");
				  // traversing my names tree
				  jrb_traverse(tmp3, p->names_tree) { 
					printf("%s ", jval_s(tmp3->val));
				
				  }  
					printf("\n");
				 }
            
        } else {
            printf("no key %s\n", str);
        }
    }
//	  dll_traverse(tmp_list2, tmp_list) {
//		p = tmp_list2->val.v;
//		free(p->addr);
//			jrb_traverse(tmp3, hosts) {
//				jrb_free_tree(p->names_tree);
//			}
//	  }

} // end of main
