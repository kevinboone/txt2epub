/*==========================================================================
  txt2epub
  text.c
  General text-handling functions
  Copyright (c)2021-24 Kevin Boone, GPL3.0 
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

// We insert into the text file a single byte that represents the
//  'verbatim' text sequence, which can be anything, of any length. The
//  default is back-tick, whose meaning bere broadly aligns with its
//  meaning in markdown. However, there's a case for using a unicode
//  character here, which means we have to provide some way to use a
//  multibyte sequence for the verbatim marker. But using a single-
//  byte marker makes subsequent processing much easier and quicker.
//  SO we convert the multi-byte marker into a single byte.
//  The single byte must be one that cannot legitmately appear in
//  UTF-8 text, like 0xC0. However, such a character _might_ appear
//  in 8-bit ASCII files. But txt2epub makes no claim to be able
//  to handle such files.
#define VERBATIM_BYTE 0xC0

static pcre *re_italic, *re_bold, *re_indent, *re_verbatim,
            *re_h1, *re_h2, *re_h3, *re_br, *re_pagenum;

/*==========================================================================
  strip_cr 
  // TODO -- remove them completely, rather than just turning them into
  //  spaces
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
  All the regular expressions we use are static, except the verbatim 
  marker, which can be set on the command line.
==========================================================================*/
void text_init_regex (const char *verbatim_marker)
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

  re_pagenum = pcre_compile ("^\\s\\s+\\d+", 0, 
    &pcreErrorStr, &pcreErrorOffset, NULL);

  re_verbatim = pcre_compile (verbatim_marker, 0, 
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
  if (re_pagenum)
    pcre_free (re_pagenum);
  if (re_verbatim)
    pcre_free (re_verbatim);
  }


/*==========================================================================
  TODO: all these 'text_subs_xxx' functions use essentially the same
  logic. Ideally, I should write a single function, with relevant
  arguments. Unfortunately, there's just enough disparity between these
  functions to make this a little awkward. 
==========================================================================*/
/*==========================================================================
  text_subs_br
==========================================================================*/
static char *text_subs_br (const char *_input)
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
static char *text_subs_indent (const char *_input)
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
static char *text_subs_italic (const char *_input)
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
static char *text_subs_h3 (const char *_input)
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
static char *text_subs_h2 (const char *_input)
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
static char *text_subs_h1 (const char *_input)
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
  text_subs_verbatim
  Replace the (maybe) multi-byte verbatim marker with the single byte
  VERBATIM_BYTE
==========================================================================*/
static char *text_subs_verbatim (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

  while (!done)
    {
    int vec[10];
    int count = pcre_exec (re_verbatim, NULL, input, strlen (input),  
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
      kmsstring_append_c (s, VERBATIM_BYTE);
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
static char *text_subs_bold (const char *_input)
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
  text_subs_pagenum
  Try to strip floating page numbers
==========================================================================*/
static char *text_subs_pagenum (const char *_input)
  {
  char *input = strdup (_input);
  BOOL done = FALSE;
  KMSString *s = kmsstring_create_empty ();

  while (!done)
    {
    int vec[10];
    int count = pcre_exec (re_pagenum, NULL, input, strlen (input),  
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
      //kmsstring_append (s, subs);
      // Do nothing in this case -- just dump the whole thing
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
  escape_html 
  For each line, we have to convert characters with a special meaning in
  XHTML to escapes. However, we don't do this for text that lies between
  verbatim markers, so we have to scan the text for these markers.
==========================================================================*/
static char *escape_html (const char *line)
  {
  KMSString *new_string = kmsstring_create ("");
  // We have to do & first, as later substitutions will insert new & chars

  unsigned char *line1 = (unsigned char *)text_subs_verbatim (line);
  unsigned char *old_line1 = line1;
  
  BOOL verbatim = FALSE;
  while (*line1)
    {
    switch (*line1)
      {
      case '&':
        if (verbatim)
          kmsstring_append_c (new_string, '&'); 
        else
          kmsstring_append (new_string, "&amp;"); 
        break;

      case '>':
        if (verbatim)
          kmsstring_append_c (new_string, '>'); 
        else
          kmsstring_append (new_string, "&gt;"); 
        break;

      case '<':
        if (verbatim)
          kmsstring_append_c (new_string, '<'); 
        else
          kmsstring_append (new_string, "&lt;"); 
        break;

      case VERBATIM_BYTE: 
        verbatim = !verbatim;
        break;

      default:
        kmsstring_append_c (new_string, *line1);
      }
    line1++;
    }

  free (old_line1);

  char *ret = strdup (kmsstring_cstr (new_string)); 
  kmsstring_destroy (new_string);
  return ret;
  }

/*==========================================================================
  format_line 
  Note -- line may (in theory) be a magabyte long
==========================================================================*/
static char *format_line (const char *line, BOOL indent_is_para, 
    BOOL markdown, BOOL remove_pagenum, BOOL first_line)
  {
  char *escaped_line = escape_html (line); 

  char *line1; 

  if (remove_pagenum)
    line1 = text_subs_pagenum (escaped_line);
  else
    line1 = strdup (escaped_line); 

  free (escaped_line);

  char *md_out;
  if (markdown)
    {
    char *line2 = text_subs_bold (line1);
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
    md_out = strdup (line1);
    }

  free (line1);

  char *line4;

  if (indent_is_para && !first_line)
    {
    // Don't process indents as para breaks if this is the first
    //   line of the file
    line4 = text_subs_indent (md_out);
    }
  else
    line4 = strdup (md_out); 

   char *line5 = strdup (line4);

  free (line4);

  free (md_out);
  return line5;
  }

/*==========================================================================
  input_file_to_html 
  If the input file is already XHTML we don't have to format it further --
    we just apply the relevant EPUB header and footer. Everthing else is
    assumed to be plain UTF8 text, which must be formated as XHTML.
==========================================================================*/
// TODO -- stdin
char *input_file_to_xhtml (const char *textfile, const char *title, 
     BOOL indent_is_para, BOOL markdown, BOOL first_is_title, BOOL line_paras,
     BOOL remove_pagenum, BOOL para_indent)
  {
  kmslog_info ("Processing file %s", textfile);

  KMSString *xml = kmsstring_create_empty();

  kmsstring_append (xml, "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n");
  kmsstring_append (xml, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
  kmsstring_append (xml, "<head>\n");
  kmsstring_append_printf (xml, "<title>%s</title>\n", title);
  kmsstring_append (xml, "</head>\n");
//add indent
  if (para_indent) 
  {
    kmsstring_append (xml, "<style>\n");
    kmsstring_append (xml, "p {\n ");
    kmsstring_append (xml, " text-indent: 1.5em;\n");
    kmsstring_append (xml, " margin-bottom: 0em;\n");
    kmsstring_append (xml, " margin-top: 0em;\n");
    kmsstring_append (xml, "}\n");
    kmsstring_append (xml, "</style>\n");
  }
///////////
  kmsstring_append (xml, "<body>\n");
  kmsstring_append (xml, "<p>\n");

  BOOL is_xhtml = FALSE;
  if (strstr (textfile, ".xhtml"))
    is_xhtml = TRUE;

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
        if (is_xhtml)
          {
	  kmsstring_append (xml, line);
          }
        else
          {
	  strip_cr (line);
	  if (strlen (line) > 1)
	    {
	    if (line[strlen(line) - 1] == 10)
	      line[strlen(line) - 1] = 0;
	    }
	  if (strlen (line) <= 1)
	    {
	    kmsstring_append (xml, "</p>\n");
	    }
	  char *newline = format_line (line, indent_is_para, markdown, 
	    remove_pagenum, (lines == 0));
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

	  if (strlen (line) <= 1)
	    {
	    kmsstring_append (xml, "<p>\n");
	    }

	  kmsstring_append (xml, "\n");
	  if (line_paras)
	    kmsstring_append (xml, "</p><p>\n");
          free (newline);
          }
        lines++;
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


