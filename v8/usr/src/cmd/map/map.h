#include <math.h>

#define PI 3.14159265358979323844
#define TWOPI (2*PI)
#define RAD (PI/180)

#define ECC .08227185422	/* eccentricity of earth */
#define EC2 .006768657997

#define FUZZ .0001

struct coord {
	float l;	/* lat or lon in radians*/
	float s;	/* sin */
	float c;	/* cos */
};
struct place {
	struct coord nlat;
	struct coord wlon;
};
double sin(), cos(), tan(), fabs(), sqrt(), log(), exp(), atan2();
double hypot();
