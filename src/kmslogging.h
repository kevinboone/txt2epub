/*==========================================================================
txt2epub
logging.h
Copyright (c)2017 Kevin Boone, GPLv3.0
*==========================================================================*/

#pragma once

#define ERROR 0
#define WARNING 1
#define INFO 2
#define DEBUG 3

#include "kmsconstants.h"

void kmslog_error (const char *fmt,...);
void kmslog_warning (const char *fmt,...);
void kmslog_info (const char *fmt,...);
void kmslog_debug (const char *fmt,...);
void kmslogging_set_level (const int level);

void kmslogging_set_log_syslog (const BOOL f);
void kmslogging_set_log_console (const BOOL f);

