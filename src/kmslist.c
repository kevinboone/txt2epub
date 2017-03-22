/*==========================================================================
kmediascanner
list.c
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <pthread.h>
#include "kmslist.h"

typedef struct _KMSListItem
  {
  struct _KMSListItem *next;
  void *data;
  } KMSListItem;

struct _KMSList
  {
  pthread_mutex_t mutex;
  KMSListItemFreeFn free_fn; 
  KMSListItem *head;
  };

/*==========================================================================
list_create
*==========================================================================*/
KMSList *kmslist_create (KMSListItemFreeFn free_fn)
  {
  KMSList *list = malloc (sizeof (KMSList));
  memset (list, 0, sizeof (KMSList));
  list->free_fn = free_fn;
  pthread_mutex_init (&list->mutex, NULL);
  return list;
  }

/*==========================================================================
kmslist_create_strings 
*==========================================================================*/
KMSList *kmslist_create_strings (void)
  {
  return kmslist_create (free);
  }

/*==========================================================================
list_destroy
*==========================================================================*/
void kmslist_destroy (KMSList *self)
  {
  if (!self) return;

  pthread_mutex_lock (&self->mutex);
  KMSListItem *l = self->head;
  while (l)
    {
    if (self->free_fn)
      self->free_fn (l->data);
    KMSListItem *temp = l;
    l = l->next;
    free (temp);
    }

  pthread_mutex_unlock (&self->mutex);
  pthread_mutex_destroy (&self->mutex);
  free (self);
  }


/*==========================================================================
list_prepend
Note that the caller must not modify or free the item added to the list. It
will remain on the list until free'd by the list itself, by calling
the supplied free function
*==========================================================================*/
void kmslist_prepend (KMSList *self, void *item)
  {
  pthread_mutex_lock (&self->mutex);
  KMSListItem *i = malloc (sizeof (KMSListItem));
  i->data = item;
  i->next = NULL;

  if (self->head)
    {
    i->next = self->head;
    self->head = i;
    }
  else
    {
    self->head = i;
    }
  pthread_mutex_unlock (&self->mutex);
  }


/*==========================================================================
list_append
Note that the caller must not modify or free the item added to the list. It
will remain on the list until free'd by the list itself, by calling
the supplied free function
*==========================================================================*/
void kmslist_append (KMSList *self, void *item)
  {
  pthread_mutex_lock (&self->mutex);
  KMSListItem *i = malloc (sizeof (KMSListItem));
  i->data = item;
  i->next = NULL;

  if (self->head)
    {
    KMSListItem *l = self->head;
    while (l->next)
      l = l->next;
    l->next = i;
    }
  else
    {
    self->head = i;
    }
  pthread_mutex_unlock (&self->mutex);
  }


/*==========================================================================
list_length
*==========================================================================*/
int kmslist_length (KMSList *self)
  {
  if (!self) return 0;

  pthread_mutex_lock (&self->mutex);
  KMSListItem *l = self->head;
  int i = 0;
  while (l != NULL)
    {
    l = l->next;
    i++;
    }

  pthread_mutex_unlock (&self->mutex);
  return i;
  }

/*==========================================================================
list_get
*==========================================================================*/
void *kmslist_get (KMSList *self, int index)
  {
  if (!self) return NULL;

  pthread_mutex_lock (&self->mutex);
  KMSListItem *l = self->head;
  int i = 0;
  while (l != NULL && i != index)
    {
    l = l->next;
    i++;
    }
  pthread_mutex_unlock (&self->mutex);

  return l->data;
  }


/*==========================================================================
list_dump
*==========================================================================*/
void kmslist_dump (KMSList *self)
  {
  int i, l = kmslist_length (self);
  for (i = 0; i < l; i++)
    {
    const char *s = kmslist_get (self, i);
    printf ("%s\n", s);
    }
  }


/*==========================================================================
list_contains
*==========================================================================*/
BOOL kmslist_contains (KMSList *self, const void *item, KMSListCompareFn fn)
  {
  if (!self) return FALSE;
  pthread_mutex_lock (&self->mutex);
  KMSListItem *l = self->head;
  BOOL found = FALSE;
  while (l != NULL && !found)
    {
    if (fn (l->data, item) == 0) found = TRUE; 
    l = l->next;
    }
  pthread_mutex_unlock (&self->mutex);
  return found; 
  }


/*==========================================================================
list_contains_string
*==========================================================================*/
BOOL kmslist_contains_string (KMSList *self, const char *item)
  {
  return kmslist_contains (self, item, (KMSListCompareFn)strcmp);
  }


/*==========================================================================
list_remove
IMPORTANT -- The "item" argument cannot be a direct reference to an
item already in the list. If that items is removed from the list its
memory will be freed. The "item" argument will this be an invalid
memory reference, and the program will crash. It is necessary
to copy the item first.
*==========================================================================*/
void kmslist_remove (KMSList *self, const void *item, KMSListCompareFn fn)
  {
  if (!self) return;
  pthread_mutex_lock (&self->mutex);
  KMSListItem *l = self->head;
  KMSListItem *last_good = NULL;
  while (l != NULL)
    {
    if (fn (l->data, item) == 0)
      {
      if (l == self->head)
        {
        self->head = l->next; // l-> next might be null
        }
      else
        {
        if (last_good) last_good->next = l->next;
        }
      self->free_fn (l->data);  
      KMSListItem *temp = l->next;
      free (l);
      l = temp;
      } 
    else
      {
      last_good = l;
      l = l->next;
      }
    }
  pthread_mutex_unlock (&self->mutex);
  }

/*==========================================================================
list_remove_string
*==========================================================================*/
void kmslist_remove_string (KMSList *self, const char *item)
  {
  kmslist_remove (self, item, (KMSListCompareFn)strcmp);
  }


/*==========================================================================
list_clone
*==========================================================================*/
KMSList *kmslist_clone (KMSList *self, KMSListCopyFn copyFn)
  {
  KMSListItemFreeFn free_fn = self->free_fn; 
  KMSList *new = kmslist_create (free_fn);

  pthread_mutex_lock (&self->mutex);
  KMSListItem *l = self->head;
  while (l != NULL)
    {
    void *data = copyFn (l->data);
    kmslist_append (new, data);
    l = l->next;
    }
  pthread_mutex_unlock (&self->mutex);

  return new;
  }



