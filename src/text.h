/*==========================================================================
kobo
text.h
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#pragma once

char *text_file_to_xhtml (const char *textfile, const char *title,
        BOOL indent_is_para, BOOL markdown);
void text_init_regex (void);
void text_cleanup_regex (void);

