//
// Created by Alessio on 7/19/2019.
//

#include "RBTree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define THREAD_CHECK(x)\
    if((x) != 0){return 0;}
#define NULL_CHECK(x)\
    if((x) == 0){return 0;}

//private functions
static void insert_fixup(node*, node**);
static void right_rotation(node*, node**);
static void left_rotation(node*, node**);
static void* rec_randsearch(node*, void*, size_t, int);
static node* rec_search(node*, void*, size_t); //returns the node if found
static void rec_print(node* n);
static size_t min(size_t, size_t);
static int void_compare(void*, void*, size_t);
void rec_write(node* n, FILE* file);
node* rec_read(node* n, FILE* file);

rb_tree* tree_init(){
    rb_tree* t = malloc(sizeof(rb_tree));
    t -> root = NULL;
    pthread_mutex_init(&t -> mutex, NULL);
    return t;
}

int tree_insert(rb_tree* tree, void* obj, size_t length){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    node* parent = NULL;
    node* x = NULL;
    node* new = NULL;
    x = rec_search(tree -> root, obj, length);
    if (x != NULL) {
        THREAD_CHECK(pthread_mutex_unlock(&tree->mutex));
        return 1; //already in tree
    }
    NULL_CHECK(new = malloc(sizeof(node)));
    new -> parent = NULL;
    new -> left = NULL;
    new -> right = NULL;
    new -> black = 1;
    new -> key = obj;
    new -> key_length = length;

    x = tree -> root;
    while(x != NULL){
        parent = x;
        if(memcmp(new -> key, x -> key, min(new -> key_length, x -> key_length)) < 0)
            x=x->left;
        else
            x=x->right;
    }
    new -> parent = parent;
    if(parent == NULL)
        tree -> root = new;
    else if(memcmp(new -> key, parent -> key, min(new -> key_length, parent -> key_length)) < 0)
        parent -> left = new;
    else 
        parent -> right = new;

    insert_fixup(new, &(tree -> root));
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    return 1;
}

static void insert_fixup(node* z,node** root){
    node* y;
    while (z -> parent != NULL && z -> parent -> black){
        if(z -> parent == z -> parent -> parent -> left){
            y = z -> parent -> parent -> right;
            if (!y || y -> black){
                z -> parent -> black = 0;
                z -> parent -> parent -> black = 1;
                z = z -> parent -> parent;
            }
            else {
                if (z == z -> parent -> right){
                    z = z -> parent;
                    left_rotation(z, root);}
                z -> parent -> black = 0;
                z -> parent -> parent -> black = 1;
                right_rotation(z -> parent -> parent, root);
            }
        }
        else {
            y = z -> parent -> parent -> left;
            if (!y || y -> black){
                z -> parent -> black = 0;
                z -> parent -> parent -> black = 1;
                z = z -> parent -> parent;
            }else{
                if(z == z -> parent -> left){
                    z = z -> parent;
                    right_rotation(z,root);
                }
                z -> parent -> black = 0;
                z -> parent -> parent -> black = 1;
                left_rotation(z -> parent -> parent, root);
            }
        }
    }
    (*root) -> black = 0;
}

static void right_rotation(node* x,node* *root){
    node* y;
    if (x -> left != NULL){
        y = x -> left;
        x -> left = y -> right;
        if(y -> right != NULL)
            y -> right -> parent = x;
        y -> parent = x -> parent;
        if (x -> parent == NULL)
            *root=y;
        else if(x == x -> parent ->right)
            x -> parent -> right = y;
        else
            x -> parent -> left = y;
        y -> right = x;
        x -> parent = y;
    }
}


static void left_rotation(node* x, node* *root){
    node* y;
    if (x -> right != NULL){
        y = x -> right;
        x -> right = y -> left;
        if(y -> left != NULL)
            y -> left -> parent = x;
        y -> parent = x -> parent;
        if(x -> parent == NULL)
            *root = y;
        else if(x == x -> parent -> left)
            x -> parent -> left = y;
        else
            x -> parent -> right = y;
        y -> left = x;
        x -> parent = y;
    }
}

size_t min(size_t s1, size_t s2){
    return s1 > s2 ? s2: s1;
}

void* tree_randsearch(rb_tree* tree, void* obj, size_t length){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    void* ret = rec_randsearch(tree -> root, obj, length, INT_MAX);
    THREAD_CHECK(pthread_mutex_unlock(&tree -> mutex));
    return ret;
}

static void* rec_randsearch(node* n, void* obj, size_t length, int previous_error){
    if (n == NULL)
        return NULL;
    int current_error = void_compare(obj, n -> key, min(n -> key_length, length));
    if(current_error > previous_error)
        return NULL;

    node *rec_node = NULL;

    int comp_result = memcmp(n->key, obj, min(n -> key_length, length));

    if (comp_result > 0){ //todo check correctness
        rec_node = rec_randsearch(n -> left, obj, length, current_error);
    }else if(comp_result < 0){
        rec_node = rec_randsearch(n -> right, obj, length, current_error);
    }

    return !rec_node ? n -> key : rec_node;
}

static node* rec_search(node* x, void* obj, size_t length){
    if(x == NULL)
        return NULL;
    int comp = memcmp(obj, x -> key, min(x -> key_length, length));
    if(comp == 0)
        return x;
    if(comp < 0)
        return rec_search(x -> left, obj, length);
    return rec_search(x -> right, obj, length);
}

int tree_print(rb_tree* tree){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    rec_print(tree -> root);
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    return 1;
}

static void rec_print(node* n){
    if(n != NULL){
        rec_print(n -> left);
        printf("%s \n", (char*) n -> key);
        rec_print(n -> right);
    }
}


int tree_save_file(rb_tree* tree, char* path){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    FILE* file = fopen(path, "w");
    NULL_CHECK(file);
    rec_write(tree -> root, file);

    THREAD_CHECK(pthread_mutex_unlock(&tree -> mutex));
    return 1;
}

void rec_write(node* n, FILE* file) {
    if (n != NULL) {
        fwrite(&n -> key_length, sizeof(size_t), 1, file);
        fwrite(n -> key, 1, n -> key_length, file);
        fwrite(&n -> black, sizeof(int), 1, file);
        rec_write(n -> left, file);
        rec_write(n -> right, file);
    }else{
        size_t s = 0;
        fwrite(&s, sizeof(size_t), 1, file); //write NULL
    }
}

int tree_load_file(rb_tree* tree, char* path){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    FILE* file = fopen(path, "r");
    NULL_CHECK(file);
    tree -> root = malloc(sizeof(node));
    fread(&tree -> root -> key_length, sizeof(size_t), 1, file);
    NULL_CHECK(tree -> root -> key = malloc(tree -> root -> key_length));
    fread(tree -> root -> key, 1, tree -> root -> key_length, file);
    tree -> root -> right = NULL;
    tree -> root -> left = NULL;
    tree -> root -> parent = NULL;
    fread(&tree -> root -> black, sizeof(int), 1, file);

    tree -> root -> left = rec_read(tree -> root, file);
    tree -> root -> right = rec_read(tree -> root, file);
    THREAD_CHECK(pthread_mutex_unlock(&tree -> mutex));
    return 1;
}


node* rec_read(node* n, FILE* file){
    node* new = malloc(sizeof(node));
    NULL_CHECK(new);
    fread(&new -> key_length, sizeof(size_t), 1, file);
    if(new -> key_length){
        NULL_CHECK(new -> key = malloc(new -> key_length));
        fread(new -> key, 1, new -> key_length, file);
        fread(&new -> black, sizeof(int), 1, file);
        new -> parent = n;
        new -> left = rec_read(new, file);
        new -> right = rec_read(new, file);
        return new;
    }else{
        free(new);
        return NULL;
    }
}

static int void_compare(void* a, void* b, size_t length) {
    int count = 0;
    for (int i = 0; i < length; i++)
        if (*(unsigned char *) a != *(unsigned char *) b)
            count++;
    return count;
}

//void tree_delete(node* z,node* *root){
//    node* y,x;
//    int yoc= z->black;
//    y=z;
//    if (z -> left == tnil){
//        x=z->right;
//        RBTransplant(z,z->right,root);
//    }
//    else if (z->right==tnil) {
//        x=z->left;
//        RBTransplant(z,z->left,root);
//    }
//    else {
//        y=treemin(z->right);
//        yoc= y->black;
//        x= y->right;
//        if (y->parent==z) x->parent=y;
//        else {
//            RBTransplant(y,y->right,root);
//            y->right=z->right;
//            y->right->parent=y;
//        }
//        RBTransplant(z,y,root);
//        y->left=z->left;
//        y->left->parent=y;
//        y->black=z->black;
//    }
//    if (yoc==0) DeleteFixup(x,root);
//    free(z->key);
//    free(z);
//}

//void DeleteFixup(node* x,node* *root){
//    node* w;
//    while (x!=*root && x->black==0){
//        if (x==x->parent->left){
//            w=x->parent->right;
//            if (w->black==1){
//                w->black=0;
//                x->parent->black=1;
//                left_rotation(x->parent,root);
//                w=x->parent->right;
//            }
//            if (w->left->black==0 && w->right->black==0){
//                w->black=1;
//                x=x->parent;
//            }
//            else {
//                if (w->right->black==0){
//                    w->left->black=0;
//                    w->black=1;
//                    right_rotation(w,root);
//                    w=x->parent->right;
//                }
//                w->black=x->parent->black;
//                x->parent->black=0;
//                w->right->black=0;
//                left_rotation(x->parent,root);
//                x=*root;
//            }
//        }
//        else{
//            w=x->parent->left;
//            if (w->black==1){
//                w->black=0;
//                x->parent->black=1;
//                right_rotation(x->parent,root);
//                w=x->parent->left;
//            }
//            if (w->right->black==0 && w->left->black==0){
//                w->black=1;
//                x=x->parent;
//            }
//            else {
//                if (w->left->black==0){
//                    w->right->black=0;
//                    w->black=1;
//                    left_rotation(w,root);
//                    w=x->parent->left;
//                }
//                w->black=x->parent->black;
//                x->parent->black=0;
//                w->left->black=0;
//                right_rotation(x->parent,root);
//                x=*root;
//            }
//        }
//    }
//    x->black=0;
//}

//void RBTransplant(node* u,node* v,node** root){
//    if (u -> parent == NULL)
//        *root = v;
//    else if(u == u -> parent -> left)
//        u -> parent -> left = v;
//    else
//        u -> parent -> right = v;
//    v -> parent = u -> parent;
//}


//static node* treemin(node* x){
//    while (x -> left != NULL)
//        x = x->left;
//    return x;
//}