#include "complex"

double c_abs(z)
complex *z;
{
double cabs();

return( cabs( z->real, z->imag ) );
}
