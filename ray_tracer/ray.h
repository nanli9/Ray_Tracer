#pragma once
#ifndef RAY

#include "vector3.h"

using point3 = vector3;


class ray
{
public:
	point3 p0;
	vector3 direction;
	ray();
	ray(point3 p0,vector3 dir);
};



#endif // !RAY
