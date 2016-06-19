// Box.h - AdVisuo Common Source File

#pragma once

#include "Vector.h"
#include <sstream>
#pragma warning( disable : 4244 )

// Simple BOX - for walls, models etc.
struct BOX
{
protected:
	AVVECTOR A;			// left front lower
	AVVECTOR B;			// right rear upper

public:
	BOX()														{ this->A = Vector(0, 0, 0); this->B = Vector(0, 0, 0); }
	BOX(AVVECTOR A, AVVECTOR B)									{ this->A = A; this->B = B; }
	BOX(AVFLOAT x, AVFLOAT y, AVFLOAT z, AVFLOAT w, AVFLOAT d, AVFLOAT h)	{ this->A = Vector(x, y, z); this->B = Vector(x + w, y + d, z + h); }
	BOX(AVVECTOR A, AVFLOAT w, AVFLOAT d, AVFLOAT h)			{ this->A = A; this->B = Vector(A.x + w, A.y + d, A.z + h); }
	BOX(AVFLOAT x, AVFLOAT y, AVFLOAT w, AVFLOAT d)				{ this->A = Vector(x, y, 0); this->B = Vector(x + w, y + d, 0); }

	operator AVVECTOR&()	{ return A; }
	BOX &operator +=(AVVECTOR v)	{ Move(v); return *this; }
	BOX &operator -=(AVVECTOR v)	{ Move(-v.x, -v.y, -v.z); return *this; }
	BOX operator +(AVVECTOR v)		{ BOX box = *this; box += v; return box; }
	BOX operator -(AVVECTOR v)		{ BOX box = *this; box -= v; return box; }
	BOX &operator *=(AVFLOAT f)		{ Scale(f); return *this; }

	BOX &operator +=(BOX box)		{ if (A.x <= B.x) { A.x = min(A.x, min(box.A.x, box.B.x)); B.x = max(B.x, max(box.B.x, box.A.x)); }
									  else		 	  { A.x = max(A.x, max(box.A.x, box.B.x)); B.x = min(B.x, min(box.B.x, box.A.x)); }
									  if (A.y <= B.y) { A.y = min(A.y, min(box.A.y, box.B.y)); B.y = max(B.y, max(box.B.y, box.A.y)); }
									  else			  { A.y = max(A.y, max(box.A.y, box.B.y)); B.y = min(B.y, min(box.B.y, box.A.y)); }
									  if (A.z <= B.z) { A.z = min(A.z, min(box.A.z, box.B.z)); B.z = max(B.z, max(box.B.z, box.A.z)); }
									  else			  { A.z = max(A.z, max(box.A.z, box.B.z)); B.z = min(B.z, min(box.B.z, box.A.z)); }
									  return *this;  }
	BOX operator+(BOX box)			{ BOX Box = *this; Box += box; return Box; }

	bool InBox(AVVECTOR &v)			{ return ((v.x >= Left() && v.x <= Right()) || (v.x <= Left() && v.x >= Right())) && ((v.y >= Front() && v.y <= Rear()) || (v.y <= Front() && v.y >= Rear())); }

	bool InWidth(AVFLOAT x)			{ return (x >= Left() && x <= Right()) || (x <= Left() && x >= Right()); }
	bool InDepth(AVFLOAT y)			{ return (y >= Front() && y <= Rear()) || (y <= Front() && y >= Rear()); }
	bool InHeight(AVFLOAT z)		{ return (z >= Lower() && z <= Upper()) || (z <= Lower() && z >= Upper()); }

	// divides the bix into nXDiv x nYDiv checkerboard-like sections and returns indices of section corresponding to the given vector
	// -1 and nXDiv/nYDiv values reserved for vectors outside the box
	bool InBoxSection(AVVECTOR &v, AVLONG nXDiv, AVLONG nYDiv, AVLONG &nX, AVLONG &nY)
	{
		nX = (AVLONG)floor((v.x - Left()) / (Width() / (double)nXDiv)); 
		nY = (AVLONG)floor((v.y - Front()) / (Depth() / (double)nYDiv));
		nX = max(-1, min(nX, nXDiv)); 
		nY = max(-1, min(nY, nYDiv));
		return nX >= 0 && nX < nXDiv && nY >= 0 && nY < nYDiv;
	}
	// azimuth from the box centre towards the given point; if normalised; corners are at PI/4 + N*PI/2 as if aspect ratio was 1.0
	AVFLOAT InBoxAzimuth(AVVECTOR &v, bool bNormalise = false)
	{
		AVFLOAT f = 1; if (bNormalise) f = 1 / Aspect();
		return atan2(f * (v.x - CentreX()), v.y - CentreY());
	}

	AVFLOAT Left()				{ return A.x; }
	AVFLOAT Right()				{ return B.x; }
	AVFLOAT Front()				{ return A.y; }
	AVFLOAT Rear()				{ return B.y; }
	AVFLOAT Lower()				{ return A.z; }
	AVFLOAT Upper()				{ return B.z; }

	AVFLOAT CentreX()			{ return (A.x + B.x) / 2; }
	AVFLOAT CentreY()			{ return (A.y + B.y) / 2; }
	AVFLOAT CentreZ()			{ return (A.z + B.z) / 2; }

	AVFLOAT Width()				{ return B.x - A.x; };
	AVFLOAT Depth()				{ return B.y - A.y; };
	AVFLOAT Height()			{ return B.z - A.z; };
	AVFLOAT Aspect()			{ return Width() / Depth(); }

	BOX SetLeft(AVFLOAT f)		{ A.x = f; return *this; }
	BOX SetRight(AVFLOAT f)		{ B.x = f; return *this; }
	BOX SetFront(AVFLOAT f)		{ A.y = f; return *this; }
	BOX SetRear(AVFLOAT f)		{ B.y = f; return *this; }
	BOX SetLower(AVFLOAT f)		{ A.z = f; return *this; }
	BOX SetUpper(AVFLOAT f)		{ B.z = f; return *this; }

	BOX SetWidth(AVFLOAT w)				{ AVFLOAT dw = w - Width(); B.x += dw; return *this; }
	BOX SetDepth(AVFLOAT d)				{ AVFLOAT dd = d - Depth(); B.y += dd; return *this; }
	BOX SetHeight(AVFLOAT h)			{ AVFLOAT dh = h - Height(); B.z += dh; return *this; }
	BOX SetAspectWidth(AVFLOAT a)		{ SetWidth(Depth() * a); return *this; }
	BOX SetAspectDepth(AVFLOAT a)		{ SetDepth(Width() / a); return *this; }

	BOX Move(AVVECTOR v)						{ A.x += v.x; A.y += v.y; A.z += v.z; B.x += v.x; B.y += v.y; B.z += v.z; return *this; }
	BOX Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ A.x += x; A.y += y; A.z += z; B.x += x; B.y += y; B.z += z; return *this; }
	BOX MoveX(AVFLOAT f)						{ A.x += f; B.x += f; return *this; }
	BOX MoveY(AVFLOAT f)						{ A.y += f; B.y += f; return *this; }
	BOX MoveZ(AVFLOAT f)						{ A.z += f; B.z += f; return *this; }
	BOX Scale(AVFLOAT f)						{ A.x *= f; A.y *= f; A.z *= f; B.x *= f; B.y *= f; B.z *= f; return *this; }
	BOX Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ A.x *= x; A.y *= y; A.z *= z; B.x *= x; B.y *= y; B.z *= z; return *this; }
	BOX ScaleX(AVFLOAT f)						{ A.x *= f; B.x *= f; return *this; }
	BOX ScaleY(AVFLOAT f)						{ A.y *= f; B.y *= f; return *this; }
	BOX ScaleZ(AVFLOAT f)						{ A.z *= f; B.z *= f; return *this; }
	BOX Grow(AVFLOAT f)							{ AVVECTOR centre = Centre(); Move(-centre); Scale(f); Move(centre); return *this; }
	BOX Grow(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ AVVECTOR centre = Centre(); Move(-centre); Scale(x, y, z); Move(centre); return *this; }
	BOX GrowX(AVFLOAT f)						{ AVVECTOR centre = Centre(); Move(-centre); ScaleX(f); Move(centre); return *this; }
	BOX GrowY(AVFLOAT f)						{ AVVECTOR centre = Centre(); Move(-centre); ScaleY(f); Move(centre); return *this; }
	BOX GrowZ(AVFLOAT f)						{ AVVECTOR centre = Centre(); Move(-centre); ScaleZ(f); Move(centre); return *this; }

	AVVECTOR LeftFrontLower()					{ return A; }
	AVVECTOR LeftFrontUpper()					{ return Vector(A.x, A.y, B.z); }
	AVVECTOR LeftRearLower()					{ return Vector(A.x, B.y, A.z); }
	AVVECTOR LeftRearUpper()					{ return Vector(A.x, B.y, B.z); }
	AVVECTOR RightFrontLower()					{ return Vector(B.x, A.y, A.z); }
	AVVECTOR RightFrontUpper()					{ return Vector(B.x, A.y, B.z); }
	AVVECTOR RightRearLower()					{ return Vector(B.x, B.y, A.z); }
	AVVECTOR RightRearUpper()					{ return B; }
	AVVECTOR CentreFrontLower()					{ return Vector((A.x + B.x) / 2, A.y, A.z); }
	AVVECTOR CentreFrontUpper()					{ return Vector((A.x + B.x) / 2, A.y, B.z); }
	AVVECTOR CentreRearLower()					{ return Vector((A.x + B.x) / 2, B.y, A.z); }
	AVVECTOR CentreRearUpper()					{ return Vector((A.x + B.x) / 2, B.y, B.z); }
	AVVECTOR CentreLower()						{ return Vector(CentreX(), CentreY(), A.z); }
	AVVECTOR CentreUpper()						{ return Vector(CentreX(), CentreY(), B.z); }
	AVVECTOR Centre()							{ return Vector(CentreX(), CentreY(), CentreZ()); }


	BOX Normalise()
	{
		if (A.x > B.x)	{ AVFLOAT t = A.x; A.x = B.x; B.x = t; }
		if (A.y > B.y)	{ AVFLOAT t = A.y; A.y = B.y; B.y = t; }
		if (A.z > B.z)	{ AVFLOAT t = A.z; A.z = B.z; B.z = t; }
		return *this;
	}

	BOX DoorPanel(AVULONG nPanelsCount, AVULONG nPanelsPerDoor, AVULONG nIndex, AVULONG nPanel, bool bLiftNotLanding, bool bFlip = false)
	{
		if (bFlip) nIndex = 1 - nIndex;
		AVULONG xDiv = nPanelsCount;
		AVULONG yDiv = nPanelsPerDoor;
		AVULONG xI = (nIndex == 0) ? nPanel : xDiv - nPanel - 1;
		AVULONG yI = bLiftNotLanding ? nPanel : yDiv - nPanel - 1;

		return BOX(LeftRearLower() + Vector(Width() * xI / xDiv, -Depth() * yI / yDiv, 0), Width() / xDiv, Depth() / yDiv, Height());
	}

	BOX DoorWithExtension(AVULONG nType, AVULONG nPanelsCount, AVULONG nPanelsPerDoor, AVFLOAT fExtra = 0, bool bFlip = false)
	{
		AVULONG xDiv = nPanelsCount;
		AVFLOAT dx = Width() / xDiv;
		BOX box = *this;

		enum DOOR_TYPE { DOOR_CENTRE_CAR = 1, DOOR_CENTRE_SHAFT = 2, DOOR_SIDE_LEFT = 3, DOOR_SIDE_RIGHT = 4 };
		if (bFlip && nType == DOOR_SIDE_LEFT) nType = DOOR_SIDE_RIGHT;
		else if (bFlip && nType == DOOR_SIDE_RIGHT) nType = DOOR_SIDE_LEFT;
		switch (nType)
		{	case DOOR_CENTRE_CAR:
			case DOOR_CENTRE_SHAFT:	
				box.SetLeft(Left() - dx); box.SetRight(Right() + dx); break;
			case DOOR_SIDE_LEFT:
				box.SetLeft(Left() - dx); box.SetRight(Right() + fExtra); break;
			case DOOR_SIDE_RIGHT:
				box.SetLeft(Left() - fExtra); box.SetRight(Right() + dx); break;
		}
		return box;
	}

	std::wstring Stringify()
	{
		std::wstringstream s;
		s << A .x << L" "<< A .y << L" "<< A .z << L" "<< B .x << L" "<< B .y << L" "<< B .z << L" ";
		return s.str();
	}

	BOX ParseFromString(std::wstring str)
	{
		std::wstringstream s(str);
		s >> A .x >> A .y >> A .z >> B .x >> B .y >> B .z;
		return *this;
	}
};

// Box with external walls (thickness)
struct XBOX : public BOX
{
private:
	AVVECTOR A1, B1;	// external points (including wall thickness)

public:
	XBOX()																	:BOX()					{ this->A1 = this->A; this->B1 = this->B; }
	XBOX(AVVECTOR A, AVVECTOR B)											:BOX(A, B)				{ this->A1 = this->A; this->B1 = this->B; }
	XBOX(AVFLOAT x, AVFLOAT y, AVFLOAT z, AVFLOAT w, AVFLOAT d, AVFLOAT h)	:BOX(x, y, z, w, d, h)	{ this->A1 = this->A; this->B1 = this->B; }
	XBOX(AVVECTOR A, AVFLOAT w, AVFLOAT d, AVFLOAT h)						:BOX(A, w, d, h)		{ this->A1 = this->A; this->B1 = this->B; }
	XBOX(AVFLOAT x, AVFLOAT y, AVFLOAT w, AVFLOAT d)						:BOX(x, y, w, d)		{ this->A1 = this->A; this->B1 = this->B; }

//	XBOX(BOX box)							:BOX(box.LeftFrontLower(), box.RightRearUpper())		{ this->A1 = this->A; this->B1 = this->B; }
//	XBOX(BOX inner, BOX outer)				:BOX(inner.LeftFrontLower(), inner.RightRearUpper())	{ this->A1 = outer.LeftFrontLower(); this->B1 = outer.RightRearUpper(); }

	BOX GetInnerBox()				{ return BOX(A, B); }
	BOX GetOuterBox()				{ return BOX(A1, B1); }
	BOX GetLargeBox()				{ return BOX(LeftExtFrontExtLower(), RightExtRearExtUpper()); }
	BOX GetTallBox()				{ return BOX(LeftFrontLowerExt(), RightRearUpperExt()); }

	operator AVVECTOR&()			{ return A; }
	XBOX &operator +=(AVVECTOR v)	{ Move(v); return *this; }
	XBOX &operator -=(AVVECTOR v)	{ Move(-v.x, -v.y, -v.z); return *this; }
	XBOX operator +(AVVECTOR v)		{ XBOX box = *this; box += v; return box; }
	XBOX operator -(AVVECTOR v)		{ XBOX box = *this; box -= v; return box; }
	XBOX &operator *=(AVFLOAT f)	{ Scale(f); return *this; }

	XBOX &operator +=(XBOX box)		{ if (A.x <= B.x) { A.x = min(A.x, min(box.A.x, box.B.x)); A1.x = min(A1.x, min(box.A1.x, box.B1.x)); B.x = max(B.x, max(box.B.x, box.A.x)); B1.x = max(B1.x, max(box.B1.x, box.A1.x)); }
									  else		 	  { A.x = max(A.x, max(box.A.x, box.B.x)); A1.x = max(A1.x, max(box.A1.x, box.B1.x)); B.x = min(B.x, min(box.B.x, box.A.x)); B1.x = min(B1.x, min(box.B1.x, box.A1.x)); }
									  if (A.y <= B.y) { A.y = min(A.y, min(box.A.y, box.B.y)); A1.y = min(A1.y, min(box.A1.y, box.B1.y)); B.y = max(B.y, max(box.B.y, box.A.y)); B1.y = max(B1.y, max(box.B1.y, box.A1.y)); }
									  else			  { A.y = max(A.y, max(box.A.y, box.B.y)); A1.y = max(A1.y, max(box.A1.y, box.B1.y)); B.y = min(B.y, min(box.B.y, box.A.y)); B1.y = min(B1.y, min(box.B1.y, box.A1.y)); }
									  if (A.z <= B.z) { A.z = min(A.z, min(box.A.z, box.B.z)); A1.z = min(A1.z, min(box.A1.z, box.B1.z)); B.z = max(B.z, max(box.B.z, box.A.z)); B1.z = max(B1.z, max(box.B1.z, box.A1.z)); }
									  else			  { A.z = max(A.z, max(box.A.z, box.B.z)); A1.z = max(A1.z, max(box.A1.z, box.B1.z)); B.z = min(B.z, min(box.B.z, box.A.z)); B1.z = min(B1.z, min(box.B1.z, box.A1.z)); }
									  return *this;  }
	XBOX operator+(XBOX box)		{ XBOX Box = *this; Box += box; return Box; }

	bool InBoxExt(AVVECTOR &v)		{ return ((v.x >= LeftExt() && v.x <= RightExt()) || (v.x <= LeftExt() && v.x >= RightExt())) && ((v.y >= FrontExt() && v.y <= RearExt()) || (v.y <= FrontExt() && v.y >= RearExt())); }

	bool InWidthExt(AVFLOAT x)		{ return (x >= LeftExt() && x <= RightExt()) || (x <= LeftExt() && x >= RightExt()); }
	bool InDepthExt(AVFLOAT y)		{ return (y >= FrontExt() && y <= RearExt()) || (y <= FrontExt() && y >= RearExt()); }
	bool InHeightExt(AVFLOAT z)		{ return (z >= LowerExt() && z <= UpperExt()) || (z <= LowerExt() && z >= UpperExt()); }

	AVFLOAT LeftExt()			{ return A1.x; }
	AVFLOAT RightExt()			{ return B1.x; }
	AVFLOAT FrontExt()			{ return A1.y; }
	AVFLOAT RearExt()			{ return B1.y; }
	AVFLOAT LowerExt()			{ return A1.z; }
	AVFLOAT UpperExt()			{ return B1.z; }

	AVFLOAT CentreXExt()		{ return (A1.x + B1.x) / 2; }
	AVFLOAT CentreYExt()		{ return (A1.y + B1.y) / 2; }
	AVFLOAT CentreZExt()		{ return (A1.z + B1.z) / 2; }

	AVFLOAT WidthExt()			{ return B1.x - A1.x; };
	AVFLOAT DepthExt()			{ return B1.y - A1.y; };
	AVFLOAT HeightExt()			{ return B1.z - A1.z; };

	AVFLOAT WidthLWall()		{ return B.x - A1.x; };
	AVFLOAT DepthFWall()		{ return B.y - A1.y; };
	AVFLOAT HeightLSlab()		{ return B.z - A1.z; };
	AVFLOAT WidthRWall()		{ return B1.x - A.x; };
	AVFLOAT DepthRWall()		{ return B1.y - A.y; };
	AVFLOAT HeightUSlab()		{ return B1.z - A.z; };

	AVFLOAT FrontThickness()	{ return A.y - A1.y; }
	AVFLOAT RearThickness()		{ return B1.y - B.y; }
	AVFLOAT LeftThickness()		{ return A.x - A1.x; }
	AVFLOAT RightThickness()	{ return B1.x - B.x; }
	AVFLOAT LowerThickness()	{ return A.z - A1.z; }
	AVFLOAT UpperThickness()	{ return B1.z - B.z; }

	XBOX SetLeftExt(AVFLOAT f)	{ A1.x = f; return *this; }
	XBOX SetRightExt(AVFLOAT f)	{ B1.x = f; return *this; }
	XBOX SetFrontExt(AVFLOAT f)	{ A1.y = f; return *this; }
	XBOX SetRearExt(AVFLOAT f)	{ B1.y = f; return *this; }
	XBOX SetLowerExt(AVFLOAT f)	{ A1.z = f; return *this; }
	XBOX SetUpperExt(AVFLOAT f)	{ B1.z = f; return *this; }

	XBOX SetWidth(AVFLOAT w)			{ AVFLOAT dw = w - Width(); B.x += dw; B1.x += dw; return *this; }
	XBOX SetDepth(AVFLOAT d)			{ AVFLOAT dd = d - Depth(); B.y += dd; B1.y += dd; return *this; }
	XBOX SetHeight(AVFLOAT h)			{ AVFLOAT dh = h - Height(); B.z += dh; B1.z += dh; return *this; }
	XBOX SetAspectWidth(AVFLOAT a)		{ SetWidth(Depth() * a); return *this; }
	XBOX SetAspectDepth(AVFLOAT a)		{ SetDepth(Width() / a); return *this; }

	XBOX SetThickness(AVFLOAT t)										{ A1 = A; A1.x -= t; A1.y -= t; A1.z -= t; B1 = B; B1.x += t; B1.y += t; B1.z += t; return *this; }
	XBOX SetThickness(AVFLOAT ltrt, AVFLOAT ftre)						{ SetThickness(ltrt, ltrt, ftre, ftre); return *this; }
	XBOX SetThickness(AVFLOAT lt, AVFLOAT rt, AVFLOAT ft, AVFLOAT re)	{ A1 = A; A1.x -= lt; A1.y -= ft; B1 = B; B1.x += rt; B1.y += re; return *this; }
	XBOX SetThickness(AVFLOAT lt, AVFLOAT rt, AVFLOAT ft, AVFLOAT re, AVFLOAT lw, AVFLOAT up)	
																		{ A1 = A; A1.x -= lt; A1.y -= ft; A1.z -= lw; B1 = B; B1.x += rt; B1.y += re; B1.z += up; return *this; }
	XBOX SetFrontThickness(AVFLOAT f)	{ A1.y = A.y - f; return *this; }
	XBOX SetRearThickness(AVFLOAT f)	{ B1.y = B.y + f; return *this; }
	XBOX SetLeftThickness(AVFLOAT f)	{ A1.x = A.x - f; return *this; }
	XBOX SetRightThickness(AVFLOAT f)	{ B1.x = B.x + f; return *this; }
	XBOX SetLowerThickness(AVFLOAT f)	{ A1.z = A.z - f; return *this; }
	XBOX SetUpperThickness(AVFLOAT f)	{ B1.z = B.z + f; return *this; }

	XBOX Move(AVVECTOR v)				{ BOX::Move(v); A1.x += v.x; A1.y += v.y; A1.z += v.z; B1.x += v.x; B1.y += v.y; B1.z += v.z; return *this; }
	XBOX Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ BOX::Move(x, y, z); A1.x += x; A1.y += y; A1.z += z; B1.x += x; B1.y += y; B1.z += z; return *this; }
	XBOX MoveX(AVFLOAT f)				{ BOX::MoveX(f); A1.x += f; B1.x += f; return *this; }
	XBOX MoveY(AVFLOAT f)				{ BOX::MoveY(f); A1.y += f; B1.y += f; return *this; }
	XBOX MoveZ(AVFLOAT f)				{ BOX::MoveX(f); A1.z += f; B1.z += f; return *this; }
	XBOX Scale(AVFLOAT f)				{ BOX::Scale(f); A1.x *= f; A1.y *= f; A1.z *= f; B1.x *= f; B1.y *= f; B1.z *= f; return *this; }
	XBOX Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ BOX::Scale(x, y, z); A1.x *= x; A1.y *= y; A1.z *= z; B1.x *= x; B1.y *= y; B1.z *= z; return *this; }
	XBOX ScaleX(AVFLOAT f)				{ BOX::ScaleX(f); A1.x *= f; B1.x *= f; return *this; }
	XBOX ScaleY(AVFLOAT f)				{ BOX::ScaleX(f); A1.y *= f; B1.y *= f; return *this; }
	XBOX ScaleZ(AVFLOAT f)				{ BOX::ScaleX(f); A1.z *= f; B1.z *= f; return *this; }
	XBOX Grow(AVFLOAT f)				{ AVVECTOR centre = Centre(); Move(-centre); Scale(f); Move(centre); return *this; }
	XBOX Grow(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ AVVECTOR centre = Centre(); Move(-centre); Scale(x, y, z); Move(centre); return *this; }
	XBOX GrowX(AVFLOAT f)				{ AVVECTOR centre = Centre(); Move(-centre); ScaleX(f); Move(centre); return *this; }
	XBOX GrowY(AVFLOAT f)				{ AVVECTOR centre = Centre(); Move(-centre); ScaleY(f); Move(centre); return *this; }
	XBOX GrowZ(AVFLOAT f)				{ AVVECTOR centre = Centre(); Move(-centre); ScaleZ(f); Move(centre); return *this; }

	AVVECTOR LeftFrontLowerExt()		{ return Vector(A.x, A.y, A1.z); }
	AVVECTOR LeftFrontExtLower()		{ return Vector(A.x, A1.y, A.z); }
	AVVECTOR LeftFrontExtLowerExt()		{ return Vector(A.x, A1.y, A1.z); }
	AVVECTOR LeftExtFrontLower()		{ return Vector(A1.x, A.y, A.z); }
	AVVECTOR LeftExtFrontLowerExt()		{ return Vector(A1.x, A.y, A1.z); }
	AVVECTOR LeftExtFrontExtLower()		{ return Vector(A1.x, A1.y, A.z); }
	AVVECTOR LeftExtFrontExtLowerExt()	{ return A1; }
	
	AVVECTOR LeftFrontUpperExt()		{ return Vector(A.x, A.y, B1.z); }
	AVVECTOR LeftFrontExtUpper()		{ return Vector(A.x, A1.y, B.z); }
	AVVECTOR LeftFrontExtUpperExt()		{ return Vector(A.x, A1.y, B1.z); }
	AVVECTOR LeftExtFrontUpper()		{ return Vector(A1.x, A.y, B.z); }
	AVVECTOR LeftExtFrontUpperExt()		{ return Vector(A1.x, A.y, B1.z); }
	AVVECTOR LeftExtFrontExtUpper()		{ return Vector(A1.x, A1.y, B.z); }
	AVVECTOR LeftExtFrontExtUpperExt()	{ return Vector(A1.x, A1.y, B1.z); }
	
	AVVECTOR LeftRearLowerExt()			{ return Vector(A.x, B.y, A1.z); }
	AVVECTOR LeftRearExtLower()			{ return Vector(A.x, B1.y, A.z); }
	AVVECTOR LeftRearExtLowerExt()		{ return Vector(A.x, B1.y, A1.z); }
	AVVECTOR LeftExtRearLower()			{ return Vector(A1.x, B.y, A.z); }
	AVVECTOR LeftExtRearLowerExt()		{ return Vector(A1.x, B.y, A1.z); }
	AVVECTOR LeftExtRearExtLower()		{ return Vector(A1.x, B1.y, A.z); }
	AVVECTOR LeftExtRearExtLowerExt()	{ return Vector(A1.x, B1.y, A1.z); }
	
	AVVECTOR LeftRearUpperExt()			{ return Vector(A.x, B.y, B1.z); }
	AVVECTOR LeftRearExtUpper()			{ return Vector(A.x, B1.y, B.z); }
	AVVECTOR LeftRearExtUpperExt()		{ return Vector(A.x, B1.y, B1.z); }
	AVVECTOR LeftExtRearUpper()			{ return Vector(A1.x, B.y, B.z); }
	AVVECTOR LeftExtRearUpperExt()		{ return Vector(A1.x, B.y, B1.z); }
	AVVECTOR LeftExtRearExtUpper()		{ return Vector(A1.x, B1.y, B.z); }
	AVVECTOR LeftExtRearExtUpperExt()	{ return Vector(A1.x, B1.y, B1.z); }
	
	AVVECTOR RightFrontLowerExt()		{ return Vector(B.x, A.y, A1.z); }
	AVVECTOR RightFrontExtLower()		{ return Vector(B.x, A1.y, A.z); }
	AVVECTOR RightFrontExtLowerExt()	{ return Vector(B.x, A1.y, A1.z); }
	AVVECTOR RightExtFrontLower()		{ return Vector(B1.x, A.y, A.z); }
	AVVECTOR RightExtFrontLowerExt()	{ return Vector(B1.x, A.y, A1.z); }
	AVVECTOR RightExtFrontExtLower()	{ return Vector(B1.x, A1.y, A.z); }
	AVVECTOR RightExtFrontExtLowerExt()	{ return Vector(B1.x, A1.y, A1.z); }
	
	AVVECTOR RightFrontUpperExt()		{ return Vector(B.x, A.y, B1.z); }
	AVVECTOR RightFrontExtUpper()		{ return Vector(B.x, A1.y, B.z); }
	AVVECTOR RightFrontExtUpperExt()	{ return Vector(B.x, A1.y, B1.z); }
	AVVECTOR RightExtFrontUpper()		{ return Vector(B1.x, A.y, B.z); }
	AVVECTOR RightExtFrontUpperExt()	{ return Vector(B1.x, A.y, B1.z); }
	AVVECTOR RightExtFrontExtUpper()	{ return Vector(B1.x, A1.y, B.z); }
	AVVECTOR RightExtFrontExtUpperExt()	{ return Vector(B1.x, A1.y, B1.z); }
	
	AVVECTOR RightRearLowerExt()		{ return Vector(B.x, B.y, A1.z); }
	AVVECTOR RightRearExtLower()		{ return Vector(B.x, B1.y, A.z); }
	AVVECTOR RightRearExtLowerExt()		{ return Vector(B.x, B1.y, A1.z); }
	AVVECTOR RightExtRearLower()		{ return Vector(B1.x, B.y, A.z); }
	AVVECTOR RightExtRearLowerExt()		{ return Vector(B1.x, B.y, A1.z); }
	AVVECTOR RightExtRearExtLower()		{ return Vector(B1.x, B1.y, A.z); }
	AVVECTOR RightExtRearExtLowerExt()	{ return Vector(B1.x, B1.y, A1.z); }
	
	AVVECTOR RightRearUpperExt()		{ return Vector(B.x, B.y, B1.z); }
	AVVECTOR RightRearExtUpper()		{ return Vector(B.x, B1.y, B.z); }
	AVVECTOR RightRearExtUpperExt()		{ return Vector(B.x, B1.y, B1.z); }
	AVVECTOR RightExtRearUpper()		{ return Vector(B1.x, B.y, B.z); }
	AVVECTOR RightExtRearUpperExt()		{ return Vector(B1.x, B.y, B1.z); }
	AVVECTOR RightExtRearExtUpper()		{ return Vector(B1.x, B1.y, B.z); }
	AVVECTOR RightExtRearExtUpperExt()	{ return B1; }

	AVVECTOR CentreFrontLowerExt()		{ return Vector((A.x + B.x) / 2, A.y, A1.z); }
	AVVECTOR CentreFrontExtLower()		{ return Vector((A.x + B.x) / 2, A1.y, A.z); }
	AVVECTOR CentreFrontExtLowerExt()	{ return Vector((A.x + B.x) / 2, A1.y, A1.z); }
	
	AVVECTOR CentreFrontUpperExt()		{ return Vector((A.x + B.x) / 2, A.y, B1.z); }
	AVVECTOR CentreFrontExtUpper()		{ return Vector((A.x + B.x) / 2, A1.y, B.z); }
	AVVECTOR CentreFrontExtUpperExt()	{ return Vector((A.x + B.x) / 2, A1.y, B1.z); }
	
	AVVECTOR CentreRearLowerExt()		{ return Vector((A.x + B.x) / 2, B.y, A1.z); }
	AVVECTOR CentreRearExtLower()		{ return Vector((A.x + B.x) / 2, B1.y, A.z); }
	AVVECTOR CentreRearExtLowerExt()	{ return Vector((A.x + B.x) / 2, B1.y, A1.z); }
	
	AVVECTOR CentreRearUpperExt()		{ return Vector((A.x + B.x) / 2, B.y, B1.z); }
	AVVECTOR CentreRearExtUpper()		{ return Vector((A.x + B.x) / 2, B1.y, B.z); }
	AVVECTOR CentreRearExtUpperExt()	{ return Vector((A.x + B.x) / 2, B1.y, B1.z); }

	AVVECTOR CentreLowerExt()			{ return Vector(CentreX(), CentreY(), A1.z); }
	AVVECTOR CentreUpperExt()			{ return Vector(CentreX(), CentreY(), B1.z); }

	AVVECTOR CentreExtLower()			{ return Vector(CentreXExt(), CentreYExt(), A.z); }
	AVVECTOR CentreExtLowerExt()		{ return Vector(CentreXExt(), CentreYExt(), A1.z); }
	AVVECTOR CentreExtUpper()			{ return Vector(CentreXExt(), CentreYExt(), B.z); }
	AVVECTOR CentreExtUpperExt()		{ return Vector(CentreXExt(), CentreYExt(), B1.z); }
	AVVECTOR CentreExt()				{ return Vector(CentreXExt(), CentreYExt(), CentreZExt()); }

	BOX LowerSlab()										{ return BOX(LeftExtRearExtLower(), WidthExt(), DepthExt(), -LowerThickness()); }
	BOX UpperSlab()										{ return BOX(LeftExtRearExtUpper(), WidthExt(), DepthExt(), UpperThickness()); }
	BOX FrontWall(AVFLOAT fShift = 0, AVFLOAT fExt = 0)	{ return BOX(LeftExtFrontLower() + Vector(fShift, 0, 0), WidthExt() - fShift + fExt, FrontThickness(), Height()); }
	BOX RearWall(AVFLOAT fShift = 0, AVFLOAT fExt = 0)	{ return BOX(RightExtRearLower() + Vector(fShift, 0, 0), WidthExt() - fShift + fExt, RearThickness(),  Height()); }
	BOX LeftWall(AVFLOAT fShift = 0, AVFLOAT fExt = 0)	{ return BOX(LeftExtFrontLower() + Vector(0, fShift, 0), Depth() - fShift + fExt, LeftThickness(), Height()); }
	BOX RightWall(AVFLOAT fShift = 0, AVFLOAT fExt = 0)	{ return BOX(RightExtRearLower() + Vector(0, fShift, 0), Depth() - fShift + fExt, RightThickness(), Height()); }

	XBOX Normalise()
	{
		if (A.x > B.x)	{ AVFLOAT t = A1.x; A1.x = B1.x; B1.x = t; }
		if (A.y > B.y)	{ AVFLOAT t = A1.y; A1.y = B1.y; B1.y = t; }
		if (A.z > B.z)	{ AVFLOAT t = A1.z; A1.z = B1.z; B1.z = t; }
		BOX::Normalise();
		return *this;
	}

	std::wstring Stringify()
	{
		std::wstringstream s;
		s << A .x << L" "<< A .y << L" "<< A .z << L" "<< B .x << L" "<< B .y << L" "<< B .z << L" "
		  << A1.x << L" "<< A1.y << L" "<< A1.z << L" "<< B1.x << L" "<< B1.y << L" "<< B1.z << L" ";
		return s.str();
	}

	XBOX ParseFromString(std::wstring str)
	{
		std::wstringstream s(str);
		s >> A .x >> A .y >> A .z >> B .x >> B .y >> B .z 
		  >> A1.x >> A1.y >> A1.z >> B1.x >> B1.y >> B1.z;
		return *this;
	}
};
