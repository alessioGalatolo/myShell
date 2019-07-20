//
// Created by Alessio on 7/19/2019.
//

#include "RBTree.h"
#include <stdlib.h>
#include <stdio.h>

#define THREAD_CHECK(x)\
    if((x) != 0){return 0;}
#define NULL_CHECK(x)\
    if((x) == 0){return 0;}

int tree_insert(rb_tree* tree, void* obj){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    node* parent = NULL;
    node* x = NULL;
    node* new = NULL;
    x = rec_search(tree -> root, obj, tree -> compare);
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

    x = tree -> root;
    while(x != NULL){
        parent = x;
        if(tree -> compare(new -> key, x -> key) < 0)
            x=x->left;
        else
            x=x->right;
    }
    new -> parent = parent;
    if(parent == NULL)
        tree -> root = new;
    else if(tree -> compare(new -> key, parent -> key) < 0)
        parent -> left = new;
    else 
        parent -> right = new;

    insert_fixup(new, &(tree -> root));
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    return 1;
}

static void insert_fixup(node* z,node* *root){
    node* y;
    while (z -> parent != NULL && z -> parent -> black){
        if(z -> parent == z -> parent -> parent -> left){
            y = z -> parent -> parent -> right;
            if (!y || y -> black){
                z -> parent -> black = 0;
                //y -> black = 0;
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
                //y -> black = 0;
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
    //tnil -> black = 0;
}

static void* rec_randsearch(node* n, void* obj, int (*compare) (void*, void*)) {
    if (n == NULL)
        return NULL;

    node *rec_node = NULL;

    if (compare(n->key, obj) > 0){ //todo check correctness
        fprintf(stderr, "looking %s, going left\n", n -> key);
        rec_node = rec_randsearch(n -> left, obj, compare);
    }else{
        fprintf(stderr, "looking %s, going right\n", n -> key);
        rec_node = rec_randsearch(n -> right, obj, compare);
    }
    if(rec_node == NULL) //is leaf
        return n -> key;
    return rec_node;
}

void* tree_randsearch(rb_tree* tree, void* obj){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    void* ret = rec_randsearch(tree -> root, obj, tree -> compare);
    THREAD_CHECK(pthread_mutex_unlock(&tree -> mutex));
    return ret;
}

static node* rec_search(node* x, void* obj, int (*compare)(void*, void*)){
    if(x == NULL)
        return NULL;
    int comp = compare(obj, x -> key);
    if(comp == 0)
        return x;
    if(comp < 0)
        return rec_search(x -> left, obj, compare);
    return rec_search(x -> right, obj, compare);
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

static void rec_print(node* n){
    if(n != NULL){
        rec_print(n -> left);
        printf("%s \n", (char*) n -> key);
        rec_print(n -> right);
    }
}

int tree_print(rb_tree* tree){
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    rec_print(tree -> root);
    THREAD_CHECK(pthread_mutex_lock(&tree -> mutex));
    return 1;
}

rb_tree* tree_init(int (*compare) (void*, void*)){
    rb_tree* t = malloc(sizeof(rb_tree));
    t -> compare = compare;
    t -> root = NULL;
    pthread_mutex_init(&t -> mutex, NULL);
    return t;
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