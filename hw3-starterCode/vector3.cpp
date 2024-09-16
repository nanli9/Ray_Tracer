#include "vector3.h"



vector3::vector3()
{
	x = 0;
	y = 0;
	z = 0;
}
vector3::vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}
vector3 vector3::operator+(const vector3& v1)
{
	vector3 ans;
	ans.x = v1.x + x;
	ans.y = v1.y + y;
	ans.z = v1.z + z;
	return ans;
}
vector3 vector3::operator*(const float& scale)
{
	vector3 ans;
	ans.x = scale * x;
	ans.y = scale * y;
	ans.z = scale * z;
	return ans;

}
float vector3::dot(vector3 v1, vector3 v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
vector3 vector3::cross(vector3 v1, vector3 v2)
{
	vector3 ans;
	
	return ans;

}
void vector3::normalize()
{
	int len_inverse = 1.0 / sqrt(x * x + y * y + z * z);
	x *= len_inverse;
	y *= len_inverse;
	z *= len_inverse;
}