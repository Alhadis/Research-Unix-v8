/* @(#) typedef.h 1.1 3/7/85 08:45:34 */
typedef short TOK;
typedef class node * PP;
typedef char bit;
typedef int (*PFI)();
typedef void (*PFV)();
typedef class node * Pnode;
typedef struct key * Pkey;
typedef class name * Pname;
typedef class basetype * Pbase;
typedef class type * Ptype;
typedef class fct  * Pfct;
typedef class field * Pfield;
typedef class expr * Pexpr;
typedef class qexpr * Pqexpr;
typedef class texpr * Ptexpr;
typedef class classdef * Pclass;
typedef class enumdef * Penum;
typedef class stmt * Pstmt;
typedef class estmt * Pestmt;
typedef class tstmt * Ptstmt;
typedef class vec * Pvec;
typedef class ptr * Pptr;
typedef class block * Pblock;
typedef class table * Ptable;
typedef class loc Loc;
typedef class call * Pcall;
typedef class gen* Pgen;
typedef class ref * Pref;
typedef class name_list * Plist;
typedef class iline * Pin;

#define forever for(;;)
