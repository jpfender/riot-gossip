#include <stdlib.h>

struct item {
   void *val;
   struct item *next;
};

typedef struct item item_t;

typedef struct list {
   item_t *head;
   size_t len;
} list_t;

item_t *list_get_head(list_t *l);
item_t *list_get_next(item_t *prev);
void list_add_item(list_t *l, void *val);
void list_remove_item(list_t *l, item_t *i);
size_t list_get_length(list_t *l);
void *list_get_value(item_t *i);
list_t *list_new();
