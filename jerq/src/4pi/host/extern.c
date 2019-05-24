int FunctionGathered, UTypeGathered, FunctionStubs, UTypeStubs;
int IdToSymCalls, StrCmpCalls;
int LexIndex;
int LexGoal;
struct Expr *CurrentExpr;
char *LexString;
char *yyerr;		/* yacc doesn't use this */
long yyres;
char Token[128];
