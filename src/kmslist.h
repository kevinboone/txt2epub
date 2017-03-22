/*==========================================================================
kmediascanner
list.h
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#pragma once

#include "kmsconstants.h"

struct _KMSList;
typedef struct _KMSList KMSList;

// The comparison function should return -1, 0, +1, like strcmp 
typedef int (*KMSListCompareFn) (const void *i1, const void *i2);
typedef void* (*KMSListCopyFn) (const void *orig);
typedef void (*KMSListItemFreeFn) (void *);

KMSList *kmslist_create (KMSListItemFreeFn free_fn);
void kmslist_destroy (KMSList *);
void kmslist_append (KMSList *self, void *item);
void kmslist_prepend (KMSList *self, void *item);
void *kmslist_get (KMSList *self, int index);
void kmslist_dump (KMSList *self);
int kmslist_length (KMSList *self);
BOOL kmslist_contains (KMSList *self, const void *item, KMSListCompareFn fn);
BOOL kmslist_contains_string (KMSList *self, const char *item);
void kmslist_remove (KMSList *self, const void *item, KMSListCompareFn fn);
void kmslist_remove_string (KMSList *self, const char *item);
KMSList *kmslist_clone (KMSList *self, KMSListCopyFn copyFn);
KMSList *kmslist_create_strings (void);

