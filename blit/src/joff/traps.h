#define HALTED  1
#define ACTIVE  2
#define TRAPPED 3
#define ERRORED 4
#define BREAKPTED 5
#define STEPPED 6
#define MODIFIED 7

#define BPTS 32		/* < 256 */
MLONG bptloc[BPTS];

#define BPT_TRAP 0
#define TRACE_TRAP 1

#define ROM 1

#define MAXTRAP 10

#define TRACK_MAX 128

