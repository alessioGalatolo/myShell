#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_SIZE 20
#define NULL_CHECK(x)\
	if((x) == NULL){perror("NULL pointer"); return -2;}

int hashcode(char* s){
	int h = 10;
	if(s == NULL)
		return -1;
	int length = strlen(s);
	for (int i = 0; i < length; i++) {
		h = 31 * h + s[i]; //stolen from java
	}
	return h;
}

hashtable_t* hash_init_wsize(int size){
	int attempts = 3;
	int i = 0;
	hashbucket** ht = NULL;
	while(i < attempts && ht == NULL)
		ht = calloc(size, sizeof(hashbucket*));
	hashtable_t* hashtable = malloc(sizeof(hashtable_t));
	if(hashtable == NULL)
		return NULL;
	hashtable -> ht = ht;
	hashtable -> size = size;
	hashtable -> nelem = 0;
	pthread_mutex_init(&(hashtable -> mutex), NULL);
	return hashtable;
}

hashtable_t* hash_init(){
	return hash_init_wsize(DEFAULT_SIZE);
}


int hash_get(hashtable_t* ht, char* s){
	if(s == NULL)
		return -2;
	int index = hashcode(s);
	pthread_mutex_lock(&(ht -> mutex));
	hashbucket* cur = (ht -> ht)[index % (ht -> size)];
	if(cur != NULL){
		do{
			if(strcmp(cur -> s, s) == 0){
				int ret = cur -> fd;
				pthread_mutex_unlock(&(ht -> mutex));
				return ret;
			}
			cur = cur -> next;
		}while(cur != NULL);
	}

	//not found
	pthread_mutex_unlock(&(ht -> mutex));
	return -1;
}

int hash_insert(hashtable_t* ht, char* s, int fd){
	if(s == NULL)
		return -2;
	if(hash_get(ht, s) == -1){//elem not found -> can be inserted
		int index = hashcode(s);
		pthread_mutex_lock(&(ht -> mutex));//lock acquire
		hashbucket* cur = (ht -> ht)[index % (ht -> size)];
		if(cur == NULL){
			NULL_CHECK((ht -> ht)[index % (ht -> size)] = malloc(sizeof(hashbucket)));
			cur = (ht -> ht)[index % (ht -> size)];
			cur -> next = NULL;
			cur -> s = NULL;

		}
		while(cur -> next != NULL){ //gets to the end of the linked list (or remains at the beginning if empty)
			cur = cur -> next;
		}
		if(cur -> s != NULL){//pointing to a element in the linked list
			NULL_CHECK(cur -> next = malloc(sizeof(hashtable_t)));
			cur = cur -> next;
			cur -> s = NULL;
			cur -> next = NULL;
			cur -> fd = 0;
		}
		//pointing to direct cell
		NULL_CHECK(cur -> s = malloc(sizeof(char) * strlen(s)));
		strcpy(cur -> s, s);
		ht -> nelem++;
		cur -> fd = fd;
		pthread_mutex_unlock(&(ht -> mutex)); //lock release
		return 0;
	}else
		return -1; //elem already in the set
}

int hash_edit(hashtable_t* ht, char* s, int fd){
	if(hash_get(ht, s) == -1)//not found -> call to insert new entry
		return hash_insert(ht, s, fd);
	//found, going to edit value of fd
	pthread_mutex_lock(&(ht -> mutex));

	int index = hashcode(s);
	hashbucket* cur = (ht -> ht)[index % (ht -> size)];
	if(cur != NULL){
		do{
			if(strcmp(cur -> s, s) == 0){
				cur -> fd = fd;
				pthread_mutex_unlock(&(ht -> mutex));
				return 0;
			}
			cur = cur -> next;
		}while(cur != NULL);
	}

	pthread_mutex_unlock(&(ht -> mutex));
	return -1;//unexpected error
}

void hash_print(hashtable_t* ht){
	pthread_mutex_lock(&(ht -> mutex));
	printf("Size: %d\tNelem: %d\n", ht -> size, ht -> nelem);
	for(int i = 0; i < ht -> size; i++){
		hashbucket* cur = ht -> ht[i];
		if(cur != NULL)
			printf("Element at index [%d] = ", i);
		else
			printf("Element at index [%d] = NULL", i);
		while(cur != NULL){
			printf("%s\t%d\t\t", cur -> s, cur -> fd);
			cur = cur -> next;
		}
		printf("\n");
	}
	pthread_mutex_unlock(&(ht -> mutex));
}

int hash_remove(hashtable_t* ht, char* s){
	if(hash_get(ht, s) != -1){ //elem is found -> can be removed
		int i = hashcode(s);
		pthread_mutex_lock(&(ht -> mutex));
		hashbucket* cur = (ht -> ht)[i];
		if(strcmp(s, cur -> s) == 0){//found in first bucket
			free(cur -> s);
			(ht -> ht)[i] = cur -> next;
			free(cur);
			pthread_mutex_unlock(&(ht -> mutex));
			return 0;
		}
		if(cur -> next != NULL){
			while(strcmp(s, cur -> next -> s) != 0){
				cur = cur -> next;
				if(cur -> next == NULL){//should not enter on correct implementation of hashtable
					pthread_mutex_unlock(&(ht -> mutex));
					return -1; //not found, bad error
				}
			}
			free(cur -> next -> s);
			hashbucket* tofree = cur -> next;
			cur -> next = cur -> next -> next;
			free(tofree);
			pthread_mutex_unlock(&(ht -> mutex));
			return 0;
		}
	}
	pthread_mutex_unlock(&(ht -> mutex));
	return -1; //not found
}

void rec_destroy(hashbucket* ht){
	if(ht != NULL){
		rec_destroy(ht -> next);

		if(ht -> s != NULL)
			free(ht -> s);
		free(ht);
	}
}


void hash_destroy(hashtable_t* ht){
	pthread_mutex_lock(&(ht -> mutex));
	for(int i = 0; i < (ht) -> size; i++){
		rec_destroy(((ht) -> ht)[i]);
		(ht -> ht)[i] = NULL;
	}
	free((ht) -> ht);
	pthread_mutex_unlock(&(ht -> mutex));
	free(ht);
}

