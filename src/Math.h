// ======================================================================== //
// Copyright 2013 Christoph Husse                                           //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "Pixel.h"

#ifndef __WIN32
#   define __forceinline
#endif

#include <vectormathlibrary/include/vectormath/cpp/vectormath_aos.h>

#define MAYBE_UNUSED

#include <sys/ref.h>
#include <common/accel.h>
#include <common/intersector.h>

#include <random>

typedef Vectormath::Aos::Matrix3 Matrix3x3;
typedef Vectormath::Aos::Matrix4 Matrix4x4;
typedef embree::Vec3f Vector3;
typedef embree::Vec4f Vector4;
typedef embree::Ray Ray;
typedef embree::Quaternion3f Quaternion;

struct VanDerCorputSequence
{
	VanDerCorputSequence();
	VanDerCorputSequence(int seed);

	double Next() const;
};

inline float operator^(const Vector3& a, const Vector3& b)
{
	return embree::dot(a, b);
}

inline Vector3 operator*(const Vector3& a, const Vector3& b)
{
	return embree::cross(a, b);
}

namespace Math
{
	static std::mt19937 randomNumberGenerator;

	static const float PI = 3.14159265359f;
	static const float PIInverse = 1/3.14159265359f;

	static Vector3 InvalidVector3() 
	{ 
		return Vector3(
			std::numeric_limits<float>::signaling_NaN(), 
			std::numeric_limits<float>::signaling_NaN(), 
			std::numeric_limits<float>::signaling_NaN()); 
	}

	static Vector3 Cross(Vector3 a, Vector3 b) { return embree::cross(a, b); }
	static float Length(Vector3 a) { return embree::length(a); }
	static float LengthSqr(Vector3 a) { return embree::length(a) * embree::length(a); }
	static Vector3 Normalized(Vector3 a) { return embree::normalize(a); }
	static float GetRandomUnitFloat() 
	{
		std::uniform_real_distribution<float> dist(0, 1);
		return dist(randomNumberGenerator);
	}

	static Quaternion RotateAroundUnit(Vector3 axis, float radians)
	{
		return embree::Quaternion3f::rotate(axis, radians);
	}

	static Vectormath::Aos::Vector3 Convert(const Vector3& vec)
	{
		return Vectormath::Aos::Vector3(vec.x, vec.y, vec.z);
	}

	static Vector3 Convert(const Vectormath::Aos::Vector3& vec)
	{
		return Vector3(vec.getX(), vec.getY(), vec.getZ());
	}

	static Vectormath::Aos::Vector4 Convert(const Vector4& vec)
	{
		return Vectormath::Aos::Vector4(vec.x, vec.y, vec.z, vec.w);
	}

	static Vector4 Convert(const Vectormath::Aos::Vector4& vec)
	{
		return Vector4(vec.getX(), vec.getY(), vec.getZ(), vec.getW());
	}

	template<class TValue>
	static TValue Clamp(TValue value, TValue min, TValue max)
	{
		return std::min(std::max(value, min), max);
	}

	static Vector3 Travel(Ray ray, float distance)
	{
		return ray.org + ray.dir * distance;
	}

	static Vector3 TransformVector(Matrix4x4 transform, Vector3 vec)
	{
		Vector4 tmp = Convert(transform * Convert(Vector4(vec.x, vec.y, vec.z, 1)));
		return Vector3(tmp.x, tmp.y, tmp.z) / tmp.w;
	}

	static Vector3 TransformVector(Matrix3x3 transform, Vector3 vec)
	{
		return Convert(transform * Convert(vec));
	}

	static Vector3 TransformDirection(Matrix3x3 transform, Vector3 vec)
	{
		return Convert(transform * Convert(vec));
	}

	static Vector3 TransformDirection(Matrix4x4 transform, Vector3 vec)
	{
		return Convert(transform.getUpper3x3() * Convert(vec));
	}

	static Vector3 Reflect(Vector3 view, Vector3 planeNormal)
	{
		return Math::Normalized(view - 2.0f * (view ^ planeNormal) / Math::LengthSqr(planeNormal) * planeNormal);
	}

	static Vector3 MinPerElem(Vector3 a, Vector3 b)
	{
		return Vector3(embree::min(a.x, b.x), embree::min(a.y, b.y), embree::min(a.z, b.z));
	}

	static Pixel MinPerElem(Pixel a, Pixel b)
	{
		return Pixel(embree::min(a.r, b.r), embree::min(a.g, b.g), embree::min(a.b, b.b));
	}

	static float MinElem(Vector3 a)
	{
		return embree::min(a.x, embree::min(a.y, a.z));
	}

	static float MaxElem(Pixel a)
	{
		return std::max(a.r, std::max(a.g, a.b));
	}

	static float MaxElem(Vector3 a)
	{
		return embree::max(a.x, embree::max(a.y, a.z));
	}

	static Vector3 MaxPerElem(Vector3 a, Vector3 b)
	{
		return Vector3(embree::max(a.x, b.x), embree::max(a.y, b.y), embree::max(a.z, b.z));
	}

	static Vector3 AbsPerElem(Vector3 a)
	{
		return Vector3(embree::abs(a.x), embree::abs(a.y), embree::abs(a.z));
	}

	static Pixel MaxPerElem(Pixel a, Pixel b)
	{
		return Pixel(embree::max(a.r, b.r), embree::max(a.g, b.g), embree::max(a.b, b.b), embree::max(a.a, b.a));
	}

	static float Sqr(float x) 
	{
		return x * x;
	}

	static float Sqrt(float x) 
	{
		if (x <= 0.0f) 
			return 0.0f;
		else 
			return std::sqrt(x);
	}

	static Vector3 DivPerElem(Vector3 a, Vector3 b)
	{
		return Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
	}

	static Pixel DivPerElem(Pixel a, Pixel b)
	{
		return Pixel(a.r / std::max(0.0001f, b.r), a.g / std::max(0.0001f, b.g), a.b / std::max(0.0001f, b.b));
	}

	static bool AreClose(float a, float b, float epsilon = 0.0001) 
	{
		// http://floating-point-gui.de/errors/comparison/
		const float absA = std::abs(a);
		const float absB = std::abs(b);
		const float diff = std::abs(a - b);

		if (a == b) 
		{ 
			// shortcut, handles infinities
			return true;
		} 
		else if ((a == 0) || (b == 0) || (diff < std::numeric_limits<float>::min()))
		{
			// a or b is zero or both are extremely close to it
			// relative error is less meaningful here
			return diff < (epsilon * std::numeric_limits<float>::min());
		} else 
		{ 
			// use relative error
			return diff / (absA + absB) < epsilon;
		}
	}

	static bool IsZero(float a, float epsilon = 0.0001)
	{
		return AreClose(a, epsilon);
	}

	static bool IsZero(Vector3 a, float epsilon = 0.0001)
	{
		return IsZero(a.x, epsilon) && IsZero(a.y, epsilon) && IsZero(a.z, epsilon);
	}

	static Vector3 GetRandomVectorInUnitQuad()
	{
		return Vector3(1 - 2 * Math::GetRandomUnitFloat(), 1 - 2 * Math::GetRandomUnitFloat(), 0);
	}

	static Vector3 GetRandomVectorInUnitCube()
	{
		return Vector3(1 - 2 * Math::GetRandomUnitFloat(), 1 - 2 * Math::GetRandomUnitFloat(), 1 - 2 * Math::GetRandomUnitFloat());
	}

	static Vector3 GetRandomVectorInUnitSphere()
	{
		// rejection sampling...
		Vector3 res;
		while(Math::Length(res = GetRandomVectorInUnitCube()) > 1);
		return res;
	}

	static Vector3 GetRandomVectorInUnitDisc()
	{
		// rejection sampling...
		Vector3 res;
		while(Math::Length(res = GetRandomVectorInUnitQuad()) > 1);
		return res;
	}

	static Vector3 GetRandomVectorInSolidAngle(Vector3 planeNormal, float maxPhi)
	{
		if(maxPhi < 0.01f)
			return planeNormal;

		// move unit disc so far along Y direction, until the hypotenuse of the discs
		// border with the frame's origin forms an angle of maxPhi/2. Then we have a cone
		// of radius maxPhi, and projected the unit disc samples along it. It is not 100%
		// what we want, but for smaller maxPhi's, which is the use-case, it is a good
		// approximation of a uniform distribution!
		float adjacentLen = 1.0f / std::tanf(maxPhi / 2);
		Vector3 adjacent = Math::Normalized(GetRandomVectorInUnitDisc() + Vector3(0,adjacentLen,0));

		// now rotate this sample into our target coordinate frame
		auto rot = Vectormath::Aos::Quat::rotation(Math::Convert(Vector3(0,1,0)), Math::Convert(planeNormal));
		return Math::Convert(Vectormath::Aos::rotate(rot, Math::Convert(adjacent)));
	} 

	static Vector3 GetRandomVectorInUnitHalfSphere(Vector3 planeNormal, float eta = 0)
	{
		// rejection sampling...
		Vector3 res;
		int i = 0;
		eta = std::min(std::max(eta, 0.0f), 0.9f);
		while((((res = GetRandomVectorInUnitSphere()) ^ planeNormal) < eta) && (i++ < 100));
		return (i < 100) ? res : planeNormal;
	} 

	static Vector3 GetOrthogonal(const Vector3& vec)
	{
		bool xZ = !Math::AreClose(vec.x, 0);
		bool yZ = !Math::AreClose(vec.y, 0);
		bool zZ = !Math::AreClose(vec.z, 0);
		Vector3 res;

		if(xZ && yZ) res = Vector3(vec.y, -vec.x, 0);
		else if(xZ && zZ) res = Vector3(vec.z, 0, -vec.x);
		else if(yZ && zZ) res = Vector3(0, vec.z, -vec.y);
		else 
			return Math::InvalidVector3();
		
		assert(AreClose(res ^ vec, 0));

		return res;
	}

	static float Saturate(float value) { return Math::Clamp(value, 0.0f, 1.0f); }
    
    template<class T>
    static T Sign(T value) {
        if(value < 0) {
            return -1;
        } else {
            return 1;
        }
    }
}



struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector3 matUv;

	Vertex() : position(0,0,0), normal(0,0,0), matUv(0,0,0) {}
};

class Mesh;
class RayTracer;
class PathSegment;

struct Triangle
{
private:
	friend RayTracer;

	int a, b, c;
	Mesh* mesh;
	int index;
	Matrix3x3 worldToUv;
	//Matrix3x3 worldToUvInverse;

public:
	const Mesh* GetMesh() const;

	// getters for AABB-Tree
	Vector3 GetPointA() const;
	Vector3 GetPointB() const;
	Vector3 GetPointC() const;
	Vector3 GetTexCoordA() const;
	Vector3 GetTexCoordB() const;
	Vector3 GetTexCoordC() const;
	Vector3 GetNormal(const PathSegment& where) const;
	Vector3 GetNormal(const Vector3& where) const;
	int GetFaceIndex() const;
	bool HasHitBackface(Vector3 worldNormal) const;

	Vector3 WorldToUv(Vector3 world) const;
	Matrix3x3 ComputeWorldToUvMatrix() const;
	float GetArea() const;
	Vector3 GetRandomPoint() const;

	Triangle(Mesh* mesh, int vertexA, int vertexB, int vertexC);

};

