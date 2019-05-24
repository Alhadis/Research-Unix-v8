#define HOSTS		"/usr/inet/lib/hosts"
#define NETWORKS	"/usr/inet/lib/networks"
#define SERVICES	"/usr/inet/lib/services"
#define EQUIV		"/usr/inet/lib/hosts.equiv"
#define GATEWAYS	"/usr/inet/lib/gateways"
#define RLOGIN		"/usr/inet/bin/rogin"
#define DKIPCONFIG	"/usr/inet/etc/dkipconfig"
#define UTMP		"/etc/utmp"
#define LOGIN		"/etc/login"

/* byte transfer routines */
extern char *memcpy();
extern char *memset();
#define bcopy(f,t,l) memcpy(t, f, l)
#define bzero(t,l) memset(t, 0, l)
