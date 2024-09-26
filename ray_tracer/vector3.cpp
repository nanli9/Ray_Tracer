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
vector3::vector3(double arr[3])
{
	this->x = (float)arr[0];
	this->y = (float)arr[1];
	this->z = (float)arr[2];
}
vector3 vector3::operator+(const vector3& v1)
{
	vector3 ans;
	ans.x = v1.x + x;
	ans.y = v1.y + y;
	ans.z = v1.z + z;
	return ans;
}
vector3 vector3::operator-(const vector3& v1)
{
	vector3 ans;
	ans.x = x - v1.x;
	ans.y = y - v1.y;
	ans.z = z - v1.z;
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
float vector3::dot(vector3 v1)
{
	return v1.x * x + v1.y * y + v1.z * z;
}
vector3 vector3::cross(vector3 v1)
{
	vector3 ans;
	ans.x = y * v1.z - z * v1.y;
	ans.y = -(x * v1.z - z * v1.x);
	ans.z = x * v1.y - y * v1.x;
	return ans;

}
void vector3::normalize()
{
	float len_inverse = 1.0 / sqrt(x * x + y * y + z * z);
	x *= len_inverse;
	y *= len_inverse;
	z *= len_inverse;
}
float vector3::len_2()
{
	return x * x + y * y + z * z;
}
vector3 vector3::multiplication(vector3 v1)
{
	vector3 ans;
	ans.x = v1.x * x;
	ans.y = v1.y * y;
	ans.z = v1.z * z;
	return ans;
}