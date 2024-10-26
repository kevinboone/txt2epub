/*==========================================================================
txt2epub
kmsstring.h
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#pragma once

#include "kmsconstants.h"

struct _KMSString;
typedef struct _KMSString KMSString;

#ifdef __cplusplus 
extern "C" {
#endif

KMSString    *kmsstring_create_empty (void);
KMSString    *kmsstring_create (const char *s);
KMSString    *kmsstring_clone (const KMSString *self);
int          kmsstring_find (const KMSString *self, const char *search);
void         kmsstring_destroy (KMSString *self);
const char   *kmsstring_cstr (const KMSString *self);
const char   *kmsstring_cstr_safe (const KMSString *self);
void         kmsstring_append_printf (KMSString *self, const char *fmt,...);
void         kmsstring_append (KMSString *self, const char *s);
void         kmsstring_append_c (KMSString *self, const char c);
void         kmsstring_prepend (KMSString *self, const char *s);
int          kmsstring_length (const KMSString *self);
KMSString    *kmsstring_substitute_all (const KMSString *self, 
                const char *search, const char *replace);
void         kmsstring_substitute_all_in_place (KMSString *self, 
                const char *search, const char *replace);
void         kmsstring_delete (KMSString *self, const int pos, 
                const int len);
void         kmsstring_insert (KMSString *self, const int pos, 
                const char *replace);
BOOL         kmsstring_create_from_utf8_file (const char *filename, 
                KMSString **result, char **error);
KMSString    *kmsstring_encode_url (const char *s);

#ifdef __cplusplus 
}
#endif




