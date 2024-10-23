/*==========================================================================
  txt2epub
  epub.c
  Copyright (c)2024 Kevin Boone, GPL3.0 
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
#include <sys/stat.h>
#include "kmsconstants.h" 
#include "kmslogging.h" 
#include "kmsstring.h" 
#include "kmslist.h" 


/*==========================================================================
  get_mime_type_by_extention 
==========================================================================*/
const char *get_mime_type_by_extension (const char *file)
  {
  const char *ret = "application/octet-stream";
  const char *p = strrchr (file, '.');
  if (p)
    {
    const char *ext = p+1;
    if (ext)
      {
      if (strcasecmp (ext, "jpg") == 0)
        ret = "image/jpeg";
      else if (strcasecmp (ext, "jpeg") == 0)
        ret = "image/jpeg";
      else if (strcasecmp (ext, "svg") == 0)
        ret = "image/svg+xml";
      else if (strcasecmp (ext, "gif") == 0)
        ret = "image/gif";
      else if (strcasecmp (ext, "png") == 0)
        ret = "image/png";
      }
    }
  return ret;
  }


/*==========================================================================
  make_toc_ncx
==========================================================================*/
char *epub_make_toc_ncx (KMSList *ch_list, const char *book_title, long pid, 
       long tim)
  {
  if (!book_title) book_title = "unknown";

  KMSString *xml = kmsstring_create_empty();

  kmsstring_append (xml, "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n");
  kmsstring_append (xml, "<ncx version=\"2005-1\" "
     "xml:lang=\"en\" xmlns=\"http://www.daisy.org/z3986/2005/ncx/\">\n");

  kmsstring_append_printf (xml, "<head><meta name=\"dtb:uid\" "
     "content=\"%08X-%04X-%04X-%04X-%04X%08X\"/><meta name=\"dtb:depth\" "
     "content=\"1\"/></head>\n",
    pid, 0, 0, 0, 0, tim);

  kmsstring_append_printf (xml, "<docTitle><text>%s</text></docTitle>", 
   book_title);

  kmsstring_append (xml, "<navMap>\n");

  int i, l = kmslist_length (ch_list);
  for (i = 0; i < l; i++)
    {
    const char *ch_name = kmslist_get (ch_list, i);
    kmsstring_append_printf (xml, "<navPoint id=\"txt2epub-%d-%ld\" "
      "playOrder=\"%d\" >\n", time(NULL), rand(), i + 1);
    kmsstring_append (xml, "<navLabel>\n");
    kmsstring_append (xml, "<text>\n");
    kmsstring_append_printf (xml, "%s", ch_name);
    kmsstring_append (xml, "</text>\n");
    kmsstring_append (xml, "</navLabel>\n");
    kmsstring_append_printf (xml, "<content src=\"file%d.html\"/>\n", i); 
    kmsstring_append (xml, "</navPoint>\n");
    }

  kmsstring_append (xml, "</navMap>\n");

  kmsstring_append (xml, "</ncx>\n");

  char *ss = strdup (kmsstring_cstr (xml));
  kmsstring_destroy (xml);
  return ss; 
  }

/*==========================================================================
  make_content_opf
==========================================================================*/
char *epub_make_content_opf (const int files, const char *title, 
     const char *author, const char *language, const char *cover_basename, 
     long pid, long time)
  {
  if (!title) title = "unknown";
  if (!author) author = "unknown";
  if (!language) language = "en";

  KMSString *xml = kmsstring_create_empty();

  kmsstring_append (xml, "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n");
  kmsstring_append (xml, "<package xmlns=\"http://www.idpf.org/2007/opf\" "
    "version=\"2.0\" unique-identifier=\"uuid_id\">\n");
  kmsstring_append (xml, "<metadata xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
    "xmlns:opf=\"http://www.idpf.org/2007/opf\" "
    "xmlns:dcterms=\"http://purl.org/dc/terms/\" "
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n");
  // TODO -- author, etc
  kmsstring_append_printf (xml, "<dc:identifier id=\"uuid_id\" "
    "opf:scheme=\"uuid\">%08X-%04X-%04X-%04X-%04X%08X</dc:identifier>\n", 
    pid, 0, 0, 0, 0, time); 
  kmsstring_append_printf (xml, "<dc:title>%s</dc:title>\n", title); 
  kmsstring_append_printf (xml, "<dc:language>%s</dc:language>\n", language); 
  kmsstring_append_printf (xml, "<dc:creator opf:role=\"aut\" "
    "opf:file-as=\"%s\">%s</dc:creator>\n", author, author); 
  kmsstring_append (xml, "</metadata>\n"); 

  kmsstring_append (xml, "<manifest>\n"); 

  if (cover_basename)
    {
    kmsstring_append (xml, 
      "<item href=\"cover.html\" id=\"cover\" media-type=\"application/xhtml+xml\"/>\n"); 
    kmsstring_append_printf (xml, 
      "<item href=\"%s\" id=\"cover_image\" media-type=\"%s\"/>\n", 
       cover_basename, get_mime_type_by_extension (cover_basename)); 
    }

  int i;
  for (i = 0; i < files; i++)
    {
    kmsstring_append_printf (xml, 
      "<item href=\"file%d.html\" id=\"file%d\" media-type=\"application/xhtml+xml\"/>\n", 
      i, i); 
    }
  kmsstring_append (xml, "<item href=\"toc.ncx\" "
    "media-type=\"application/x-dtbncx+xml\" id=\"ncx\"/>\n");
  kmsstring_append (xml, "</manifest>\n"); 

  kmsstring_append (xml, "<spine toc=\"ncx\">\n"); 

  if (cover_basename) 
    {
    kmsstring_append  (xml, "<itemref idref=\"cover\"/>\n");
    }
  for (i = 0; i < files; i++)
    {
    kmsstring_append_printf  (xml, "<itemref idref=\"file%d\"/>\n", i);
    }
  kmsstring_append (xml, "</spine>\n"); 
  kmsstring_append (xml, "</package>\n"); 


  char *ss = strdup (kmsstring_cstr (xml));
  kmsstring_destroy (xml);
  return ss; 
  }

/*==========================================================================
  make_container_xml 
==========================================================================*/
char *epub_make_container_xml (void)
  {
  return strdup ("<?xml version=\"1.0\"?><container version=\"1.0\" "
     "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">"
     "<rootfiles><rootfile full-path=\"content.opf\" "
     "media-type=\"application/oebps-package+xml\"/></rootfiles></container>");
  }


/*==========================================================================
  epub_make_cover 
==========================================================================*/
char *epub_make_cover (const char *cover_image) 
  {
  kmslog_info ("Creating cover page");

  KMSString *xml = kmsstring_create_empty();

  kmsstring_append (xml, "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n");
  kmsstring_append (xml, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
  kmsstring_append (xml, "<head>\n");
  kmsstring_append_printf (xml, "<title>Cover</title>\n");
  kmsstring_append (xml, "</head>\n");
  kmsstring_append (xml, "<body>\n");
  kmsstring_append (xml, "<p>\n");
  kmsstring_append_printf (xml, "<img src=\"%s\" alt=\"cover\"/>\n", 
       cover_image);
  kmsstring_append (xml, "</p>\n");
  kmsstring_append (xml, "</body>\n");
  kmsstring_append (xml, "</html>\n");

  char *ss = strdup (kmsstring_cstr (xml));
  kmsstring_destroy (xml);
  return ss; 

  }



