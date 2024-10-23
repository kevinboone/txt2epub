/*==========================================================================
kobo
epub.h
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#pragma once

char *epub_make_toc_ncx (KMSList *ch_list, const char *book_title, long pid, 
       long time);
char *epub_make_content_opf (const int files, const char *title, 
     const char *author, const char *language, const char *cover_basename, 
     long pid, long time);
char *epub_make_container_xml (void);
char *epub_make_cover (const char *cover_image);

