// Vector.h - AdVisuo Common Source File

#pragma once

#define __FW(x)						*(FWVECTOR*)&(x)
#define __FW_Vector					*(FWVECTOR*)&Vector

inline AVVECTOR Vector(AVFLOAT x, AVFLOAT y = 0, AVFLOAT z = 0)
{
	AVVECTOR v = {x, y, z };
	return v;
}

inline AVVECTOR VectorCross(const AVVECTOR &a, const AVVECTOR &b)
{
	AVVECTOR v = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	return v;
}

inline AVFLOAT VectorDot(const AVVECTOR &a, const AVVECTOR &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline AVVECTOR VectorAdd(const AVVECTOR &a, const AVVECTOR &b)
{
	AVVECTOR v = { a.x + b.x, a.y + b.y, a.z + b.z };
	return v;
}

inline AVVECTOR VectorSub(const AVVECTOR &a, const AVVECTOR &b)
{
	AVVECTOR v = { a.x - b.x, a.y - b.y, a.z - b.z };
	return v;
}

inline AVVECTOR VectorNeg(const AVVECTOR &a)
{
	AVVECTOR v = { -a.x, -a.y, -a.z };
	return v;
}

inline AVVECTOR VectorMul(const AVVECTOR &a, AVFLOAT f)
{
	AVVECTOR v = { a.x * f, a.y * f, a.z * f };
	return v;
}

inline AVFLOAT VectorLen(const AVVECTOR &a)
{
	return (AVFLOAT)sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline void VectorNormalise(AVVECTOR &a)
{
	AVFLOAT fLen = VectorLen(a);
	a.x /= fLen; a.y /= fLen; a.z /= fLen;
}

inline AVVECTOR VectorNormalisedCross(const AVVECTOR &a, const AVVECTOR &b)
{
	AVVECTOR v = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	VectorNormalise(v);
	return v;
}

inline AVVECTOR operator+ (AVVECTOR &v1, AVVECTOR &v2)
{
	return VectorAdd(v1, v2);
}

inline AVVECTOR operator- (AVVECTOR &v1, AVVECTOR &v2)
{
	return VectorSub(v1, v2);
}

inline AVVECTOR operator- (AVVECTOR &v)
{
	return VectorNeg(v);
}

inline AVVECTOR operator* (AVFLOAT f, AVVECTOR &v)
{
	return VectorMul(v, f);
}

inline AVVECTOR operator* (AVVECTOR &v, AVFLOAT f)
{
	return VectorMul(v, f);
}
