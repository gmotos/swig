/* ----------------------------------------------------------------------------- 
 * misc.c
 *
 *     Miscellaneous functions that don't really fit anywhere else.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

static char cvsroot[] = "$Header$";

#include "swig.h"
#include "swigver.h"
#include <ctype.h>

/* -----------------------------------------------------------------------------
 * Swig_copy_string()
 *
 * Duplicate a NULL-terminate string given as a char *.
 * ----------------------------------------------------------------------------- */

char *
Swig_copy_string(const char *s) {
  char *c = 0;
  if (s) {
    c = (char *) malloc(strlen(s)+1);
    strcpy(c,s);
  }
  return c;
}

/* -----------------------------------------------------------------------------
 * Swig_banner()
 *
 * Emits the SWIG identifying banner.
 * ----------------------------------------------------------------------------- */

void
Swig_banner(File *f) {
  Printf(f,
"/* ----------------------------------------------------------------------------\n\
 * This file was automatically generated by SWIG (http://www.swig.org).\n\
 * Version %s %s\n\
 * \n\
 * This file is not intended to be easily readable and contains a number of \n\
 * coding conventions designed to improve portability and efficiency. Do not make\n\
 * changes to this file unless you know what you are doing--modify the SWIG \n\
 * interface file instead. \n\
 * ----------------------------------------------------------------------------- */\n\n", SWIG_VERSION, SWIG_SPIN);

}

/* -----------------------------------------------------------------------------
 * Swig_section()
 * 
 * Print a comment denoting a section of wrapper code 
 * ----------------------------------------------------------------------------- */

void Swig_section(File *f, const String_or_char *name) {
  Printf(f,"/* -----------------------------------------------------------------------------\n");
  Printf(f," * %s\n", name);
  Printf(f," * ----------------------------------------------------------------------------- */\n");
}

/* -----------------------------------------------------------------------------
 * Swig_temp_result()
 *
 * This function is used to return a "temporary" result--a result that is only
 * guaranteed to exist for a short period of time.   Typically this is used by
 * functions that return strings and other intermediate results that are
 * used in print statements.
 *
 * Note: this is really a bit of a kludge to make it easier to work with
 * temporary variables (so that the caller doesn't have to worry about
 * memory management).   In theory, it is possible to break this if an
 * operation produces so many temporaries that it overflows the internal
 * array before they are used.   However, in practice, this would only
 * occur for very deep levels of recursion or functions taking lots of
 * parameters---neither of which occur very often in SWIG (if at all).
 * Also, a user can prevent destruction of a temporary object by increasing
 * it's reference count using DohIncref().
 *
 * It is an error to place two identical results onto this list.  It is also
 * an error for a caller to free anything returned by this function.
 *
 * Note: SWIG1.1 did something similar to this in a less-organized manner.
 * ----------------------------------------------------------------------------- */

#define MAX_RESULT  512

static DOH       *results[MAX_RESULT];
static int        results_index = 0;
static int        results_init = 0;

DOH  *Swig_temp_result(DOH *x) {
  int i;
  if (!results_init) {
    for (i = 0; i < MAX_RESULT; i++) results[i] = 0;
    results_init = 1;
  }
  /*  Printf(stdout,"results_index = %d, %x, '%s'\n", results_index, x, x); */
  if (results[results_index]) Delete(results[results_index]);
  results[results_index] = x;
  results_index = (results_index + 1) % MAX_RESULT;
  return x;
}
  

/* -----------------------------------------------------------------------------
 * Swig_string_escape()
 *
 * Takes a string object and produces a string with escape codes added to it.
 * ----------------------------------------------------------------------------- */

String *Swig_string_escape(String *s) {
  String *ns;
  int c;
  ns = NewString("");
  
  while ((c = Getc(s)) != EOF) {
    if (c == '\n') {
      Printf(ns,"\\n");
    } else if (c == '\r') {
      Printf(ns,"\\r");
    } else if (c == '\t') {
      Printf(ns,"\\t");
    } else if (c == '\\') {
      Printf(ns,"\\\\");
    } else if (c == '\'') {
      Printf(ns,"\\'");
    } else if (c == '\"') {
      Printf(ns,"\\\"");
    } else if (c == ' ') {
      Putc(c,ns);
    } else if (!isgraph(c)) {
      Printf(ns,"\\0%o", c);
    } else {
      Putc(c,ns);
    }
  }
  return ns;
}
     
/* -----------------------------------------------------------------------------
 * Swig_string_mangle()
 * 
 * Take a string and mangle it by stripping all non-valid C identifier characters
 * ----------------------------------------------------------------------------- */

String *Swig_string_mangle(String *s) {
  String *t = Copy(s);
  char *c = Char(t);
  while (*c) {
    if (!isalnum(*c)) *c = '_';
    c++;
  }
  return t;
}

/* -----------------------------------------------------------------------------
 * Swig_proto_cmp()
 *
 * Compares a function prototype against an expected type-string.
 * For example, Swig_proto_cmp("f(p.void,p.Tcl_Interp,int,p.p.char).int", node)
 * ----------------------------------------------------------------------------- */

int
Swig_proto_cmp(const String_or_char *pat, DOH *node) {
  SwigType *ty;
  SwigType *ct;
  ParmList  *p;
  int       r;

  ty = Gettype(node);
  p = Getparms(node);
  if (!ty || !p) return -1;
  ct = Copy(ty);
  SwigType_add_function(ct,p);
  SwigType_strip_qualifiers(ct);
  r = Cmp(pat,ct);
  Delete(ct);
  return r;
}


/* -----------------------------------------------------------------------------
 * Swig_init()
 *
 * Initialize the SWIG core
 * ----------------------------------------------------------------------------- */

void
Swig_init() {
  DohEncoding("escape", Swig_string_escape);
  Swig_typemap_init();

}
