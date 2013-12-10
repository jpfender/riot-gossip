#include "list.h"

item_t *list_get_head(list_t *l) {
    if(l == 0) return 0;
    mutex_lock(l->lock);
    item_t *tmp = l->head;
    mutex_unlock(l->lock);
    return tmp;
}

item_t *list_get_next(item_t *prev) {
    return prev->next;
}

void list_add_item(list_t *l, void *val) {
    if(val == 0 || l == 0) return;

    mutex_lock(l->lock);
    if(l->head == 0) {
        l->head = malloc(sizeof(item_t));
        l->head->val = val;
        l->head->next = 0;
    }
    else {
        item_t *cur = l->head;
        while(!(cur->next == 0)) {
            cur = cur->next;
        }
        cur->next = malloc(sizeof(item_t));
        cur->next->next = 0;
        cur->next->val = val;
    }
    l->len++;
    mutex_unlock(l->lock);
}

void list_remove_item(list_t *l, item_t *i) {
    if(l->head == 0 || l == 0) return;

    mutex_lock(l->lock);
    if(l->head == i) {
        if(l->head->next) {
            item_t *tmp = l->head->next;
            free(l->head->val);
            free(l->head);
            l->head = tmp;
        }
        else {
            free(l->head->val);
            free(l->head);
            l->head = 0;
        }
    }
    else {
        item_t *cur = l->head->next;
        item_t *last = l->head;
        while(!(cur == i)) {
            last = cur;
            cur = cur->next;
        }
        if(!cur) {
            mutex_unlock(l->lock);
            return;
        }
        last->next = cur->next;
        free(cur->val);
        free(cur);
    }
    l->len--;
    mutex_unlock(l->lock);
}

size_t list_get_length(list_t *l){
    mutex_lock(l->lock);
    size_t tmp = l->len;
    mutex_unlock(l->lock);
    return tmp;
}

void *list_get_value(item_t *i) {
    return i->val;
}


list_t *list_new(){
    list_t *new = malloc(sizeof(list_t));
    new->head = 0;
    new->len = 0;
    new->lock = malloc(sizeof(struct mutex_t));
    mutex_init(new->lock);
    return new;
}

void list_free(list_t *l) {
    if(l->head == 0 || l == 0) {
        free(l);
        return;
    }

    item_t *cur = l->head;
    item_t *next = 0;
    while(!(cur->next == 0)) {
            next = cur->next;
            free(cur->val);
            free(cur);
            cur = next;
    }
    free(l->lock);
    free(l);
}

item_t *list_get_item_by_value(list_t *l, void* v) {
    mutex_lock(l->lock);
    if(l->head == 0 || l == 0) return NULL;

    item_t *cur = l->head;
    while(cur) {
        if(cur->val == v) {
            mutex_unlock(l->lock);
            return cur;
        }
        cur = cur->next;
    }
    mutex_unlock(l->lock);
    return NULL;
}
