#include "list.h"

item_t *list_get_head(list_t *l) {
    if(l == 0) return 0;
    return l->head;
}

item_t *list_get_next(item_t *prev) {
    return prev->next;
}

void list_add_item(list_t *l, void *val) {
    if(val == 0 || l == 0) return;

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
}

void list_remove_item(list_t *l, item_t *i) {
    if(l->head == 0 || l == 0) return;

    if(l->head == i) {
        free(l->head);
        l->head = 0;
    }
    else {
        item_t *cur = l->head->next;
        item_t *last = l->head;
        while(!(cur->next == i)) {
            last = cur;
            cur = cur->next;
        }
        last->next = cur->next;
        free(cur);
    }
    l->len--;
}

size_t list_get_length(list_t *l){
    return l->len;
}

void *list_get_value(item_t *i) {
    return i->val;
}


list_t *list_new(){
    list_t *new = malloc(sizeof(list_t));
    return new;
}
