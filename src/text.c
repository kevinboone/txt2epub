/*==========================================================================
  txt2epub
  text.c
  Copyright *c)201 Kevin Boone, GPL3.0 
==========================================================================*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <pcre.h>
#include <sys/stat.h>
#include "kmsconstants.h" 
#include "kmslogging.h" 
#include "kmsstring.h" 
#include "kmslist.h" 
#include "text.h" 

static pcre *re_italic, *re_bold, *re_indent,
            *re_h1, *re_h2, *re_h3, *re_br;

/*==========================================================================
  strip_cr 
  // TODO -- remove them completely
==========================================================================*/
static void strip_cr (char *s)
  {
  while (*s)
    {
    if (*s == 13) *s = ' ';
    s++;
    }
  }


/*==========================================================================
  text_init_regex 
==========================================================================*/

void text_init_regex(void)
  {
   const char *pcreErrorStr;
  int pcreErrorOffset = 0;

  re_italic = pcre_compile ("_.*?_", PCRE_EXTENDED, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_bold = pcre_compile ("\\*.*?\\*", PCRE_EXTENDED, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_indent = pcre_compile ("^\\s\\s\\s+", PCRE_EXTENDED, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_h1 = pcre_compile ("^#.*$", 0, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_h2 = pcre_compile ("^##.*$", 0, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_h3 = pcre_compile ("^###.*$", 0, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_br = pcre_compile ("  $", 0, 
    &pcreErrorStr, &pcreErrorOffset, NULL);
  }


/*==========================================================================
  text_cleanup_regex 
==========================================================================*/
void text_cleanup_regex(void)
  {
  if (re_italic)
    pcre_free (re_italic);
  if (re_bold)
    pcre_free (re_bold);
  if (re_indent)
    pcre_free (re_indent);
  if (re_h1)
    pcre_free (re_h1);
  if (re_h2)
    pcre_free (re_h2);
  if (re_h3)
    pcre_free (re_h3);
  if (re_br)
    pcre_free (re_br);
  }


/*==========================================================================
  text_subs_br
==========================================================================*/
char *text_subs_br (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

while (!done)
  {
  int vec[10];
  int count = pcre_exec (re_br, NULL, input, strlen (input),  
     0, 0, vec, 10);         

  if (count != 1) done = TRUE;
  if (!done)
    {
    char *temp = strdup (input);
    temp[vec[0]] = 0;
    kmsstring_append (s, temp);
    free (temp);
    char *subs = strdup (input+ vec[0]+1);
    subs [vec[1] - vec[0] - 2] = 0;
    kmsstring_append (s, "<br/>");
    free (subs);
    temp = strdup (input + vec[1]);
    free (temp);
    memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
    }
  }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }



/*==========================================================================
  text_subs_indent
==========================================================================*/
char *text_subs_indent (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

while (!done)
  {
  int vec[10];
  int count = pcre_exec (re_indent, NULL, input, strlen (input),  
     0, 0, vec, 10);         

  if (count != 1) done = TRUE;
  if (!done)
    {
    char *temp = strdup (input);
    temp[vec[0]] = 0;
    kmsstring_append (s, temp);
    free (temp);
    char *subs = strdup (input+ vec[0]+1);
    subs [vec[1] - vec[0] - 2] = 0;
    kmsstring_append (s, "</p>");
    kmsstring_append (s, "");
    kmsstring_append (s, "<p>");
    free (subs);
    temp = strdup (input + vec[1]);
    free (temp);
    memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
    }
  }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }



/*==========================================================================
  text_subs_italic
==========================================================================*/
char *text_subs_italic (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

while (!done)
  {
  int vec[10];
  int count = pcre_exec (re_italic, NULL, input, strlen (input),  
     0, 0, vec, 10);         

  if (count != 1) done = TRUE;
  if (!done)
    {
    char *temp = strdup (input);
    temp[vec[0]] = 0;
    kmsstring_append (s, temp);
    free (temp);
    char *subs = strdup (input+ vec[0]+1);
    subs [vec[1] - vec[0] - 2] = 0;
    kmsstring_append (s, "<i>");
    kmsstring_append (s, subs);
    kmsstring_append (s, "</i>");
    free (subs);
    temp = strdup (input + vec[1]);
    free (temp);
    memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
    }
  }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }


/*==========================================================================
  text_subs_h3
==========================================================================*/
char *text_subs_h3 (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

  while (!done)
    {
    int vec[10];
    int count = pcre_exec (re_h3, NULL, input, strlen (input),  
       0, 0, vec, 10);         

    if (count != 1) done = TRUE;
    if (!done)
      {
      char *temp = strdup (input);
      temp[vec[0]] = 0;
      kmsstring_append (s, temp);
      free (temp);
      char *subs = strdup (input + vec[0]+3);
      subs [vec[1] - vec[0] - 3] = 0;
      kmsstring_append (s, "<h3>");
      kmsstring_append (s, subs);
      kmsstring_append (s, "</h3>");
      free (subs);
      temp = strdup (input + vec[1]);
      free (temp);
      memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
      }
    }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }

/*==========================================================================
  text_subs_h2
==========================================================================*/
char *text_subs_h2 (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

  while (!done)
    {
    int vec[10];
    int count = pcre_exec (re_h2, NULL, input, strlen (input),  
       0, 0, vec, 10);         

    if (count != 1) done = TRUE;
    if (!done)
      {
      char *temp = strdup (input);
      temp[vec[0]] = 0;
      kmsstring_append (s, temp);
      free (temp);
      char *subs = strdup (input + vec[0]+2);
      subs [vec[1] - vec[0] - 2] = 0;
      kmsstring_append (s, "<h2>");
      kmsstring_append (s, subs);
      kmsstring_append (s, "</h2>");
      free (subs);
      temp = strdup (input + vec[1]);
      free (temp);
      memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
      }
    }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }

/*==========================================================================
  text_subs_h1
==========================================================================*/
char *text_subs_h1 (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

  while (!done)
    {
    int vec[10];
    int count = pcre_exec (re_h1, NULL, input, strlen (input),  
       0, 0, vec, 10);         

    if (count != 1) done = TRUE;
    if (!done)
      {
      char *temp = strdup (input);
      temp[vec[0]] = 0;
      kmsstring_append (s, temp);
      free (temp);
      char *subs = strdup (input + vec[0]+1);
      subs [vec[1] - vec[0] - 1] = 0;
      kmsstring_append (s, "<h1>");
      kmsstring_append (s, subs);
      kmsstring_append (s, "</h1>");
      free (subs);
      temp = strdup (input + vec[1]);
      free (temp);
      memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
      }
    }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }


/*==========================================================================
  text_subs_bold
==========================================================================*/
char *text_subs_bold (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

while (!done)
  {
  int vec[10];
  int count = pcre_exec (re_bold, NULL, input, strlen (input),  
     0, 0, vec, 10);         

  if (count != 1) done = TRUE;
  if (!done)
    {
    char *temp = strdup (input);
    temp[vec[0]] = 0;
    kmsstring_append (s, temp);
    free (temp);
    char *subs = strdup (input+ vec[0]+1);
    subs [vec[1] - vec[0] - 2] = 0;
    kmsstring_append (s, "<b>");
    kmsstring_append (s, subs);
    kmsstring_append (s, "</b>");
    free (subs);
    temp = strdup (input + vec[1]);
    free (temp);
    memmove (input, input + vec[1], strlen (input) - vec[1] + 1);
    }
  }

  kmsstring_append (s, input);

  char *t = strdup (kmsstring_cstr(s));
  free (input);
  kmsstring_destroy (s);
  return t;
  }


/*==========================================================================
  format_line 
  // Note -- line may (in theory) be a magabyte long
==========================================================================*/
static char *format_line (const char *line, BOOL indent_is_para, 
    BOOL markdown)
  {
  char *md_out;
  if (markdown)
    {
    char *line2 = text_subs_bold (line);
    char *line3 = text_subs_italic (line2);
    free (line2);
    char *line4 = text_subs_h3 (line3);
    free (line3);
    char *line5 = text_subs_h2 (line4);
    free (line4);
    char *line6 = text_subs_h1 (line5);
    free (line5);
    char *line7 = text_subs_br (line6);
    free (line6);
    md_out = line7;
    }
  else
    {
    md_out = strdup (line);
    }
  char *line4;
  if (indent_is_para)
    line4 = text_subs_indent (md_out);
  else
    line4 = strdup (md_out); 
  free (md_out);
  return line4;
  }

/*==========================================================================
  textfile_to_html 
==========================================================================*/
// TODO -- stdin
char *text_file_to_xhtml (const char *textfile, const char *title, 
     BOOL indent_is_para, BOOL markdown, BOOL first_is_title)
  {
  kmslog_info ("Processing file %s", textfile);

  KMSString *xml = kmsstring_create_empty();

  kmsstring_append (xml, "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n");
  kmsstring_append (xml, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
  kmsstring_append (xml, "<head>\n");
  kmsstring_append_printf (xml, "<title>%s</title>\n", title);
  kmsstring_append (xml, "</head>\n");
  kmsstring_append (xml, "<body>\n");
  kmsstring_append (xml, "<p>\n");

  FILE *f;
  if (strcmp (textfile, "-") == 0)
    f = stdin;
  else
    f = fopen (textfile, "r");
  if (f)
    {
    BOOL done = FALSE;
    int lines = 0;
    do
      {
      size_t n = 0;
      char *line = NULL;
      if (getline (&line, &n, f) < 0) done = TRUE;
      if (!done)
        {
        strip_cr (line);
        if (strlen (line) > 1)
          {
          if (line[strlen(line) - 1] == 10)
            line[strlen(line) - 1] = 0;
          }
        if (strlen (line) <= 1)
          {
          kmsstring_append (xml, "</p><p>\n");
          }
        char *newline = format_line (line, indent_is_para, markdown);
        if (first_is_title && (lines == 0))
          {
          kmsstring_append (xml, "<h1>");
          kmsstring_append (xml, newline);
          kmsstring_append (xml, "</h1>");
          }
        else
          {
          kmsstring_append (xml, newline);
          }
        kmsstring_append (xml, "\n");
        lines++;
        free (newline);
        } 
      if (line) free (line);
      } while (!done); 
    fclose (f);
    }
  else
    {
    kmsstring_append_printf (xml, "Can't read file %s", textfile);
    kmslog_error ("Can't read file: %s", textfile);
    }
  
  kmsstring_append (xml, "</p>\n");
  kmsstring_append (xml, "</body>\n");
  kmsstring_append (xml, "</html>\n");

  char *ss = strdup (kmsstring_cstr (xml));
  kmsstring_destroy (xml);
  return ss; 
  }



