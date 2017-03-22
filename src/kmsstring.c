/*==========================================================================
kmediascanner
kmsstring.c
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "kmsstring.h"

struct _KMSString
  {
  char *str;
  }; 


/*==========================================================================
kmsstring_create_empty 
*==========================================================================*/
KMSString *kmsstring_create_empty (void)
  {
  return kmsstring_create ("");
  }


/*==========================================================================
kmsstring_create
*==========================================================================*/
KMSString *kmsstring_create (const char *s)
  {
  KMSString *self = malloc (sizeof (KMSString));
  self->str = strdup (s);
  return self;
  }


/*==========================================================================
kmsstring_destroy
*==========================================================================*/
void kmsstring_destroy (KMSString *self)
  {
  if (self)
    {
    if (self->str) free (self->str);
    }
  free (self);
  }


/*==========================================================================
kmsstring_cstr
*==========================================================================*/
const char *kmsstring_cstr (const KMSString *self)
  {
  return self->str;
  }


/*==========================================================================
kmsstring_cstr_safe
*==========================================================================*/
const char *kmsstring_cstr_safe (const KMSString *self)
  {
  if (self)
    {
    if (self->str) 
      return self->str;
    else
      return "";
    }
  else
    return "";
  }


/*==========================================================================
kmsstring_append
*==========================================================================*/
void kmsstring_append (KMSString *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = strlen (self->str) + strlen (s) + 2;
  self->str = realloc (self->str, newlen);
  strcat (self->str, s);
  }


/*==========================================================================
kmsstring_prepend
*==========================================================================*/
void kmsstring_prepend (KMSString *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = strlen (self->str) + strlen (s) + 2;
  char *temp = strdup (self->str); 
  free (self->str);
  self->str = malloc (newlen);
  strcpy (self->str, s);
  strcat (self->str, temp);
  free (temp);
  }


/*==========================================================================
kmsstring_append_printf
*==========================================================================*/
void kmsstring_append_printf (KMSString *self, const char *fmt,...) 
  {
  if (self->str == NULL) self->str = strdup ("");
  va_list ap;
  va_start (ap, fmt);
  char *s;
  vasprintf (&s, fmt, ap);
  kmsstring_append (self, s);
  free (s);
  va_end (ap);
  }


/*==========================================================================
kmsstring_length
*==========================================================================*/
int kmsstring_length (const KMSString *self)
  {
  if (self == NULL) return 0;
  if (self->str == NULL) return 0;
  return strlen (self->str);
  }


/*==========================================================================
kmsstring_clone
*==========================================================================*/
KMSString *kmsstring_clone (const KMSString *self)
  {
  if (!self) return NULL;
  if (!self->str) return kmsstring_create_empty();
  return kmsstring_create (kmsstring_cstr (self));
  }


/*==========================================================================
kmsstring_find
*==========================================================================*/
int kmsstring_find (const KMSString *self, const char *search)
  {
  if (!self) return -1;
  if (!self->str) return -1;
  const char *p = strstr (self->str, search);
  if (p)
    return p - self->str;
  else
    return -1;
  }


/*==========================================================================
kmsstring_delete
*==========================================================================*/
void kmsstring_delete (KMSString *self, const int pos, const int len)
  {
  char *str = self->str;
  if (pos + len > strlen (str))
    kmsstring_delete (self, pos, strlen(str) - len);
  else
    {
    char *buff = malloc (strlen (str) - len + 2);
    strncpy (buff, str, pos); 
    strcpy (buff + pos, str + pos + len);
    free (self->str);
    self->str = buff;
    }
  }


/*==========================================================================
kmsstring_insert
*==========================================================================*/
void kmsstring_insert (KMSString *self, const int pos, 
    const char *replace)
  {
  char *buff = malloc (strlen (self->str) + strlen (replace) + 2);
  char *str = self->str;
  strncpy (buff, str, pos);
  buff[pos] = 0;
  strcat (buff, replace);
  strcat (buff, str + pos); 
  free (self->str);
  self->str = buff;
  }



/*==========================================================================
kmsstring_substitute_all
*==========================================================================*/
KMSString *kmsstring_substitute_all (const KMSString *self, 
    const char *search, const char *replace)
  {
  KMSString *working = kmsstring_clone (self);
  BOOL cont = TRUE;
  while (cont)
    {
    int i = kmsstring_find (working, search);
    if (i >= 0)
      {
      kmsstring_delete (working, i, strlen (search));
      kmsstring_insert (working, i, replace);
      }
    else
      cont = FALSE;
    }
  return working;
  }


/*==========================================================================
  kmsstring_create_from_utf8_file 
*==========================================================================*/
BOOL kmsstring_create_from_utf8_file (const char *filename, 
    KMSString **result, char **error)
  {
  KMSString *self = NULL;
  BOOL ok = FALSE; 
  int f = open (filename, O_RDONLY);
  if (f > 0)
    {
    self = malloc (sizeof (KMSString));
    struct stat sb;
    fstat (f, &sb);
    int64_t size = sb.st_size;
    char *buff = malloc (size + 2);
    read (f, buff, size);
    self->str = buff; 
    self->str[size] = 0;
    *result = self;
    ok = TRUE;
    }
  else
    {
    asprintf (error, "Can't open file '%s' for reading: %s", 
      filename, strerror (errno));
    ok = FALSE;
    }

  return ok;
  }


/*==========================================================================
  kmsstring_encode_url
*==========================================================================*/
static char to_hex(char code)
  {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
  }


KMSString *kmsstring_encode_url (const char *str)
  {
  if (!str) return kmsstring_create_empty();;
  const char *pstr = str; 
  char *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr)
    {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_'
      || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4),
         *pbuf++ = to_hex(*pstr & 15);
    pstr++;
    }
  *pbuf = '\0';
  KMSString *result = kmsstring_create (buf);
  free (buf);
  return (result);
  }




