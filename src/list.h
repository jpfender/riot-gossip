/** \file list.h
 *  \brief Linked list implementation
 */

#include <stdlib.h>
#include "mutex.h"

/** This struct hold the value of an item and the next item in the list
 */
struct item {
   /** Pointer to value of item */
   void *val;
   /** Pointer to next item in list */
   struct item *next;
};

typedef struct item item_t;

/** This struct holds the defining elements of a linked list
 */
typedef struct list {
   /** Pointer to the first item of the list */
   item_t *head;
   /** Number of elements in list */
   size_t len;
   /** Mutex to allow concurrent access to the list */
   struct mutex_t* lock;
} list_t;

/** \brief get first item of the list
 *  \param l list to work with
 *  \return first item of the list or NULL if list is empty
 */
item_t *list_get_head(list_t *l);

/** \brief get next item in list
 *  \param prev current item
 *  \return next item or NULL if given item was last in list
 */
item_t *list_get_next(item_t *prev);

/** \brief add an item to the end of the list
 *  \param l list to work on
 *  \param val value of the item to add
 */
void list_add_item(list_t *l, void *val);

/** \brief remove given item from list
 *  \param l list to work on
 *  \param i item to remove
 */
void list_remove_item(list_t *l, item_t *i);

/** \brief get current number of items in list
 *  \param l list to work on
 *  \return number of items in list
 */
size_t list_get_length(list_t *l);

/** \brief get pointer to value of an item
 *  \param i item of interest
 *  \return pointer to value of item
 */
void *list_get_value(item_t *i);

/** \brief create a new linked list
 *  \return pointer to new linked list
 */
list_t *list_new(void);

/** \brief frees a list
 *  \param l list to free
 */
void list_free(list_t *l);

/** \brief find item in list based on the value pointer
 *  \param l list to work on
 *  \param v pointer to item value
 *  \return pointer to item if value is found, else NULL
 */
item_t *list_get_item_by_value(list_t *l, void* v);
