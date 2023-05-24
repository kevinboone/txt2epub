/*==========================================================================
  txt2epub
  main.c
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
#include "epub.h" 
#include "text.h" 


/*==========================================================================
  string_to_file 
==========================================================================*/
int string_to_file (const char *str, const char *file)
  {
  int ret = 0;
  int f = open (file, O_WRONLY | O_CREAT | O_TRUNC, 0755);
  if (f >= 0)
    {
    write (f, str, strlen (str));
    close (f);
    }
  else 
    {
    ret = errno;
    printf ("%s\n", strerror (errno));
    }
  return ret;
  }


/*==========================================================================
file_get_first_line
Read the first line in a file. Returns NULL if the file cannot be read
==========================================================================*/
char *file_get_first_line (char *filename)
  {
  char *ret = NULL;
  FILE *f = fopen (filename, "r");
  if (f)
    {
    char *line;
    size_t n = 0;
    if (getline (&line, &n, f) > 0) 
      ret = line;
    fclose (f);
    }
  return ret;
  }
 

/*==========================================================================
make_chapter_list
Use the filenames as a list of chapter names
==========================================================================*/
KMSList *make_chapter_list (char **argv, int argc, int offset, 
    BOOL firstlines)
  {
  KMSList *ch_list = kmslist_create_strings();

  if (firstlines)
    {
    int i;
    for (i = offset; i < argc; i++)
      {
      char *ch = file_get_first_line (argv[i]); 
      if (!ch)
        {
        char *filename = basename (argv[i]);
        ch = strdup (filename);
        char *p = strrchr (ch, '.');
        if (p) *p = 0;
        }
      kmslist_append (ch_list, ch); 
      }
    }
  else
    {
    int i;
    for (i = offset; i < argc; i++)
      {
      char *filename = basename (argv[i]);
      char *ch = strdup (filename);
      char *p = strrchr (ch, '.');
      if (p) *p = 0;
      kmslist_append (ch_list, ch); 
      }
    }

  return ch_list;
  }


/*==========================================================================
  main
==========================================================================*/
int main (int argc, char **argv)
  {
  static BOOL show_version = FALSE;
  static BOOL show_usage = FALSE;
  static BOOL firstlines = FALSE;
  static BOOL extra_para = FALSE;
  static BOOL para_indent = FALSE;
  static BOOL remove_pagenum = FALSE;
  static int loglevel = ERROR;
  char *epub_file = NULL;
  char *book_title = NULL;
  char *book_author = NULL;
  char *book_language = NULL;
  char *cover_image = NULL;
  char *cover_basename = NULL; 

  static struct option long_options[] = 
   {
     {"author", required_argument, NULL, 'a'},
     {"cover-image", required_argument, NULL, 'c'},
     {"first-lines", no_argument, &firstlines, 'f'},
     {"para-indent", no_argument, NULL, 0},
     {"help", no_argument, &show_usage, '?'},
     {"loglevel", required_argument, NULL, 0},
     {"output-file", required_argument, NULL, 'o'},
     {"ignore-indent", no_argument, NULL, 'i'},
     {"ignore-markdown", no_argument, NULL, 'm'},
     {"remove-pagenum", required_argument, NULL, 'r'},
     {"title", required_argument, NULL, 't'},
     {"extra-para", no_argument, NULL, 'x'},
     {"version", no_argument, &show_version, 'v'},
     {0, 0, 0, 0}
   };

  BOOL indent_is_para = TRUE;
  BOOL markdown = TRUE;

  int opt;
  while (1)
   {
   int option_index = 0;
   opt = getopt_long (argc, argv, "vhp?o:t:a:l:imc:fxr",
     long_options, &option_index);

   if (opt == -1) break;

   switch (opt)
     {
     case 0:
        if (strcmp (long_options[option_index].name, "version") == 0)
          show_version = TRUE;
        else if (strcmp (long_options[option_index].name, "first-lines") == 0)
          firstlines = TRUE;
        else if (strcmp (long_options[option_index].name, "remove_pagenum") 
             == 0)
          remove_pagenum = TRUE;
        else if (strcmp (long_options[option_index].name, "help") == 0)
          show_usage = TRUE;
        else if (strcmp (long_options[option_index].name, "loglevel") == 0)
          loglevel = atoi (optarg);
        else if (strcmp (long_options[option_index].name, "output-file") == 0)
          epub_file = strdup (optarg);
        else if (strcmp (long_options[option_index].name, "title") == 0)
          book_title = strdup (optarg);
        else if (strcmp (long_options[option_index].name, "author") == 0)
          book_author = strdup (optarg);
        else if (strcmp (long_options[option_index].name, "cover-image") == 0)
          cover_image = strdup (optarg);
        else if (strcmp (long_options[option_index].name, "language") == 0)
          book_language = strdup (optarg);
        else if (strcmp (long_options[option_index].name, "ignore-index") == 0)
          indent_is_para = FALSE; 
        else if (strcmp (long_options[option_index].name, "extra-para") == 0)
          extra_para = TRUE; 
        else if (strcmp (long_options[option_index].name, "para-indent") == 0)
          para_indent = TRUE; 
        else if (strcmp (long_options[option_index].name, "ignore-markdown") 
               == 0)
          markdown = FALSE; 
        else
          exit (-1);
        break;

     case 'a': book_author = strdup (optarg); break;
     case 'c': cover_image = strdup (optarg); break;
     case 'f': firstlines = TRUE; break;
     case 'i': indent_is_para = FALSE; break;
     case 'l': book_language = strdup (optarg); break;
     case 'o': epub_file = strdup (optarg); break;
     case 'p': para_indent = TRUE; break;
     case 'm': markdown = FALSE; break;
     case 'r': remove_pagenum = TRUE; break;
     case 't': book_title = strdup (optarg); break;
     case 'v': show_version = TRUE; break;
     case 'x': extra_para = TRUE; break;
     case '?': show_usage = TRUE; break;
     default:  exit(-1);
     }
   }

  if (show_usage)
    {
    printf ("Usage %s [options]\n", argv[0]);
    printf ("  -a,--author A         set book author (default: unknown)\n");
    printf ("  -c,--cover-image F    use image file F as the cover\n");
    printf ("     --loglevel N       log verbosity, 0 (default) - 3\n");
    printf ("  --ignore-indent       don't break paragraph on indent\n");
    printf ("  --ignore-markdown     do not respect Markdown formatting\n");
    printf ("  -f,--first-lines      first line is chapter heading\n");
    printf ("  -l,--language A       set book language (default: en)\n");
    printf ("  -r,--remove-pagenum   try to remove page numbers\n");
    printf ("  -t,--title A          set book title (default: filename)\n");
    printf ("  -v,--version          show version information\n");
    printf ("  -o,--output-file      EPUB output filename\n");
    printf ("  -p,--para-indent      Paragraph indent replaces blank line\n");
    printf ("  -x,--extra-para       Every input line is a paragraph\n");
    printf ("  -?                    show this message\n");
    exit (0);
    }
 
  if (show_version)
    {
    printf ("txt2epub " VERSION "\n");
    printf ("Copyright (c)2017 Kevin Boone and other contributors\n");
    printf ("Distributed according to the terms of the GPL, v3.0\n");
    exit (0);
    }

  int ret = 0;
  kmslogging_set_level (loglevel); 


  int file_count = argc - optind; 
  if (file_count > 0)
    {
    if (epub_file)
      {
      // We alread know the name of the output file
      } 
    else
      {
      if (file_count == 1)
	{
        const char *input_file = argv [optind];
	if (strcmp (input_file, "-") == 0)
	  {
	  // Need to specify an output filename with input stdin
	  kmslog_error ("Output file (-o) must be specified when input is stdin");
	  ret = -1;
	  }
	else
	  {
	  // Use the input filename as a base for the output filename
	  epub_file = malloc (strlen (input_file) + 20);
          strcpy (epub_file, input_file);
	  char *p = strrchr (epub_file, '.');
	  if (p)
	    {
            *p = 0;
	    }
          strcat (epub_file, ".epub");
	  }
	}
      else
	{
	// Need to specify an output filename
	kmslog_error 
	  ("Output file (-o) must be specified with multiple input files");
	ret = -1;
	}
      }
    }
  else
    {
    kmslog_error ("No input files specified");
    ret = -1;
    } 

  if (ret == 0)
    {
    text_init_regex();

    // At this point the output filename is known, so we can use it as
    //   the book title, unless a title is specified

    if (book_title)
      {
      // Do nothing -- we already know it
      }
    else
      {
      book_title = strdup (basename (epub_file));
      char *p = strrchr (book_title, '.');
      if (p) *p = 0;
      kmslog_debug ("Book title \"%s\" derived from output filename", 
       book_title);
      }

    int pid = getpid();

    char *working_dir;
    asprintf (&working_dir, "/tmp/txt2epub%d",pid); 

    kmslog_debug ("Working directory is %s", working_dir);
    if (mkdir (working_dir, 0755) == 0)
      {
      char *meta_inf;
      asprintf (&meta_inf, "%s/META-INF", working_dir);
      mkdir (meta_inf, 0755);

      char *container;
      asprintf (&container, "%s/container.xml", meta_inf);
      char *container_xml = epub_make_container_xml();
      char *cmd;
      ret = string_to_file (container_xml, container);

      // Copy the cover image, if there is one
      if (cover_image)
        {
        if (access (cover_image, R_OK) == 0)
          {
          char *cmd;
          asprintf (&cmd, "cp \"%s\" \"%s\"", cover_image, working_dir);
          system (cmd);
          free (cmd);
          }
        else
          {
          kmslog_error ("Can't read cover image file: %s", cover_image);
          }
        cover_basename = basename (cover_image);
        }
 
      if (ret == 0)
	{
        // This sucks. We need the absolute pathname of the epub file
        //   because the zip utilituy reqires that we cd to the temp dir;
        //   but the epub file does not yet exist. So we have to create it
        //   so that realpath will work. And then we have to delete it again,
        //   else zip -r will fail, because it isn't a proper zipfile. Ugh.
        int f = open (epub_file, O_WRONLY | O_CREAT | O_TRUNC);
        if (f > 0)
          {
	  close (f);
	  char epub_path[1024];
	  realpath (epub_file, epub_path); 
	  unlink (epub_path);
	  char *content;
	  asprintf (&content, "%s/content.opf", working_dir);
	  char *content_opf = epub_make_content_opf (file_count, book_title,
            book_author, book_language, cover_basename); 
	  ret = string_to_file (content_opf, content);
	  if (ret == 0)
	    { 
	    char *mimetype;
	    asprintf (&mimetype, "%s/mimetype", working_dir);
	    string_to_file ("application/epub+zip", mimetype);
	    free (mimetype);
 
            KMSList *chapter_list = make_chapter_list (argv, argc, optind, 
              firstlines); 
	     
	    char *tocncx;
	    char *tocncx_ncx = epub_make_toc_ncx (chapter_list, book_title); 
	    asprintf (&tocncx, "%s/toc.ncx", working_dir);
	    string_to_file (tocncx_ncx, tocncx);
	    free (tocncx);
	    free (tocncx_ncx);
	     
	    char *cover;
	    char *cover_xhtml = epub_make_cover (cover_basename); 
	    asprintf (&cover, "%s/cover.html", working_dir);
	    string_to_file (cover_xhtml, cover);
	    free (cover);
	    free (cover_xhtml);

	    int i;
	    for (i = 0; i < file_count; i++)
	      {
	      char *file;
              char *title = kmslist_get (chapter_list, i);
	      asprintf (&file, "%s/file%d.html", working_dir, i);
	      char *file_html = text_file_to_xhtml (argv [optind+i], title,
                indent_is_para, markdown, firstlines, extra_para, 
                remove_pagenum, para_indent);
	      if (string_to_file (file_html, file))
                {
                kmslog_error 
                  ("Can't write file %s: %s\n", file, strerror(errno));
                }
	      free (file);
	      free (file_html);
	      }

	    kmslog_debug ("Creating zipfile %s", epub_path);

	    asprintf (&cmd, "cd \"%s\"; zip -q -r \"%s\" .", 
              working_dir, epub_path);
	    system (cmd);
	    free (cmd);

            kmslist_destroy (chapter_list);
	    }
	  free (content);
	  free (content_opf);
          }
        else
          {
          ret = errno;
	  kmslog_error 
            ("Can't write output file %s: %s", epub_file, strerror (errno));
          }
	}
      else
	{
	kmslog_error ("Can't write file %s: %s", container_xml, strerror (errno));
	}
      free (container_xml);
      kmslog_debug ("Deleting temporary directory %s", working_dir);
      asprintf (&cmd, "rm -rf \"%s\"", working_dir);
      system (cmd);
      free (cmd);
      free (container);
      free (meta_inf);
      }
    else
      {
      kmslog_error ("Can't create temporary directory %s: %s",
	working_dir, strerror (errno));
      ret = errno; 
      }

    free (working_dir);
    text_cleanup_regex();
    }

  if (epub_file) free (epub_file);
  if (book_title) free (book_title);
  if (book_author) free (book_author);
  if (book_language) free (book_language);
  // I don't understand why I don't need to free at least one of the
  //  following, but valgrind says not
  //if (cover_image) free (cover_image);
  //if (cover_basename) free (cover_basename);

  return ret;
  }


