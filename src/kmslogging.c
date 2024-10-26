/*===========================================================================
epub2txt
logging.c
Copyright (c)2017 Kevin Boone, GPL v3.0
===========================================================================*/

#define _GNU_SOURCE 1
#include <syslog.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "kmsconstants.h"
#include "kmslogging.h"


static int log_level = DEBUG;
static BOOL log_syslog = TRUE;
static BOOL log_console = TRUE;

/*==========================================================================
logging_set_level
*==========================================================================*/
void kmslogging_set_level (const int level)
  {
  log_level = level;
  }

/*==========================================================================
logging_set_log_syslog
*==========================================================================*/
void kmslogging_set_log_syslog (const BOOL f)
  {
  log_syslog = f;
  }

/*==========================================================================
logging_set_log_console
*==========================================================================*/
void kmslogging_set_log_console (const BOOL f)
  {
  log_console = f;
  }

/*==========================================================================
level_to_text
*==========================================================================*/
static const char *level_to_text (const int level)
  {
  const char *ret = "ERROR";
  if (level == DEBUG) ret = "DEBUG";
  else if (level == WARNING) ret = "WARNING";
  else if (level == INFO) ret = "INFO";
  return ret;
  }

/*==========================================================================
log_vprintf
*==========================================================================*/
void kmslog_vprintf (const int level, const char *fmt, va_list ap)
  {
  if (log_console)
    {
    if (level > log_level) return;
    char *str = NULL;
    vasprintf (&str, fmt, ap);
    printf ("%s %s\n", level_to_text (level), str);
    free (str);
    }
  }


/*==========================================================================
log_error
*==========================================================================*/
void kmslog_error (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  kmslog_vprintf (ERROR,  fmt, ap);
  va_end (ap);
  if (log_syslog)
    {
    va_start (ap, fmt);
    vsyslog (LOG_ERR, fmt, ap);
    va_end (ap);
    }
  }


/*==========================================================================
kmslog_warn
*==========================================================================*/
void kmslog_warning (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  kmslog_vprintf (WARNING,  fmt, ap);
  va_end (ap);
  if (log_syslog)
    {
    va_start (ap, fmt);
    vsyslog (LOG_WARNING, fmt, ap);
    va_end (ap);
    }
  }


/*==========================================================================
kmslog_info
*==========================================================================*/
void kmslog_info (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  kmslog_vprintf (INFO,  fmt, ap);
  va_end (ap);
  if (log_syslog)
    {
    va_start (ap, fmt);
    vsyslog (LOG_INFO, fmt, ap);
    va_end (ap);
    }
  }


/*==========================================================================
kmslog_debug
*==========================================================================*/
void kmslog_debug (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  kmslog_vprintf (DEBUG,  fmt, ap);
  va_end (ap);
  }



