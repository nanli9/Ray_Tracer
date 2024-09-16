#pragma once

#ifndef VECTOR_3

#include <math.h>

class vector3
{
public:
	float x, y, z;
	vector3();
	vector3(float x, float y, float z);
	vector3 operator+(const vector3& v1);
	vector3 operator*(const float& scale);
	float dot(vector3 v1, vector3 v2);
	vector3 cross(vector3 v1, vector3 v2);
	void normalize();
};


#endif // !VECTOR_3
