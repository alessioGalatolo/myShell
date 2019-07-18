#if !defined(_HASHTABLE_H_)
#define _HASHTABLE_H_

#include <pthread.h>

//a simple hashtable. It handles confilcts with linked lists
typedef struct _hashbucket{
	char* s;//key
	int fd;//value
	struct _hashbucket* next;
}hashbucket;

typedef struct{
	pthread_mutex_t mutex;
	int size;
	int nelem;
	hashbucket** ht;
}hashtable_t;

hashtable_t* hash_init();
hashtable_t* hash_init_wsize(int);
int hash_get(hashtable_t*, char*);
int hash_insert(hashtable_t*, char*, int);
int hash_edit(hashtable_t*, char*, int);
int hash_remove(hashtable_t*, char*);
void hash_print(hashtable_t*);
void hash_destroy(hashtable_t*);


#endif
