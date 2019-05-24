/*
 *	       D U M P L D . H
 *
 * Structures used by dumpld, a debugging line-discipline.
 * These structures are also referenced by dumpcatch, which takes
 * output from dumpld and sends it to a file.
 *
 *
 * Written by Kurt Gollhardt  (Nirvonics, Inc.)
 * Last update Fri Mar 29 17:18:35 1985
 *
 */

struct dumpld {
     char      *base;
     char      *fillp;
     int       size;
};

struct dumpinf {
     dev_t     dev;
};

struct dumpheader {
     short     count;
     char      type;
     char      class;
};
