// Lab 6 - JMalloc Lab
// 10/25/21
// Zach Hamm
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "mymalloc.h"

typedef struct Chunk {
	size_t size;
	struct Chunk *next;
} chunk;

chunk *head = NULL;

int compare(const void *p1, const void *p2) {
	return (*(int*)p1 - *(int*)p2);
}

void *my_malloc(size_t size) {

	bool greater = false;
	size_t temp_size = (size + 7 + 8) & -8; // Rounds size up, we need 8 bytes
	size_t mem_size = 8192;

	if (temp_size > mem_size) 
		greater = true;

	if (head == NULL) { 
		if(greater)
			mem_size = temp_size;

		head = sbrk(mem_size);
		head->size = mem_size;
		head->next = NULL;
	}
		if(greater)
			mem_size = temp_size;

    // Search for free list node with enough available space. If not found,
	// create a new node with sbrk().
	
	// search for the chunk pointer with enough space, if you don't find one, create and link new pointer
	chunk *free_ptr = head;

	while (free_ptr->size < temp_size) {
		
		if(free_ptr->next == NULL) {
			free_ptr->next = sbrk(mem_size);
			free_ptr = free_ptr->next;
			free_ptr->size = mem_size;

		} else {
			free_ptr = free_ptr->next;
		}

	}

	if (free_ptr->size >= temp_size + 8) {
		// Enough memory for another allocation

		// allocate memory
		char *tmp_ptr = (char*)free_ptr;
		tmp_ptr += (free_ptr->size - temp_size);

		free_ptr->size -= temp_size;

		// typecast so I'm not adding 8 chunk structs, and im adding 8 bytes
		*((int *) tmp_ptr) = temp_size;	
		return (void *) (tmp_ptr + 8);
	
	} else {
		// Not enough memory for another allocation
		// give user all the bytes instead of splitting the chunk
		*((int *) free_ptr) = temp_size;

		if (free_ptr == head) { 
			head = head->next;

		} else {
			// loop through the list
			chunk *temp = head;

			while (temp->next != free_ptr) {
				temp = temp->next;
			}

			temp->next = temp->next->next;
		}

		return ((void *) free_ptr) + 8;
	}
} // end of mymalloc()

void my_free(void *ptr) {
	// return the chunk of memory to the free list
	//
	
	chunk *temp;
	temp = ptr - 8;


	if(head != NULL) {
		temp->next = head->next;
		head->next = temp;
	}
	else {
		head = (chunk *) temp;
	}
}
 
// returns the start of the free list unless head is null then returns null
void *free_list_begin() {
	if(head == NULL)
		return NULL;

	return head;
}

void *free_list_next(void *node) {
	// Return a pointer to the next node on the free list
	chunk *temp;
	temp = node;
	if(temp->next == NULL)
		return NULL;

	return temp->next;

}

void coalesce_free_list() {
	// process my free list and combine all the adjacent entries 
	
	// Count nodes in free list.
	chunk *curr_node = head;
	chunk *temp_node = head;
	chunk *temp_node2 = head;

	int counter = 0;

	while (1) {
		if(curr_node == NULL)
			break;
		else {
		counter++;
		curr_node = curr_node->next;
		}
	}
	// array to be sorted
	void **nodes_sorted = malloc(BUFSIZ);

	for(unsigned int i = 0; i < counter; i++) {
		nodes_sorted[i] = (void *)temp_node;
		temp_node = temp_node->next;
	}

	// Sort node list in order.


	qsort(nodes_sorted, counter, sizeof(void *), compare);

	for(unsigned int k = 0; k < counter; k++) {

		((chunk *) nodes_sorted[k])->next = nodes_sorted[k+1];

		if(k == (counter - 1))
			((chunk *) nodes_sorted[k])->next = NULL;
	}
		
	
	head = (chunk *) nodes_sorted[0];
	((chunk *) nodes_sorted[counter - 1])->next = NULL;

	curr_node = head;

	// Traverse free list, checking following nodes.
	while (1) {
		if(curr_node == NULL)
			break;
		else {
			temp_node = curr_node->next;

			while (1) {
				if(((void*)curr_node) + curr_node->size != temp_node) 
					break;
				else {
					curr_node->next = temp_node->next;
					curr_node->size = temp_node->size + curr_node->size;
					temp_node = temp_node->next;

				}

			}

			curr_node = curr_node->next;
	  }

	}

}
