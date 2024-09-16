#include "ray.h"


ray::ray()
{
	
}

ray::ray(point3 p0, vector3 dir)
{
	this->p0 = p0;
	this->direction = dir;
}