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


#include "stdafx.h"

const Mesh* Triangle::GetMesh() const { return mesh; }
Vector3 Triangle::GetPointA() const { return mesh->vertices[a].position; }
Vector3 Triangle::GetPointB() const { return mesh->vertices[b].position; }
Vector3 Triangle::GetPointC() const { return mesh->vertices[c].position; }
Vector3 Triangle::GetTexCoordA() const { return mesh->vertices[a].matUv; }
Vector3 Triangle::GetTexCoordB() const { return mesh->vertices[b].matUv; }
Vector3 Triangle::GetTexCoordC() const { return mesh->vertices[c].matUv; }
int Triangle::GetFaceIndex() const { return index; }

float Triangle::GetArea() const
{
	return 0.5f * Math::Length(Math::Cross(GetPointC() - GetPointA(), GetPointC() - GetPointB()));
}

bool Triangle::HasHitBackface(Vector3 worldNormal) const 
{ 
	return (worldNormal ^ mesh->vertices[a].normal) > 0; 
}

Vector3 Triangle::GetRandomPoint() const
{
	/*
		Uniform random distribution on triangle's surface.
		Source: http://www.cs.princeton.edu/~funk/tog02.pdf
			Chapter: Shape Distributions (page 814)
	*/

	float r1 = std::sqrtf(Math::GetRandomUnitFloat());
	float r2 = Math::GetRandomUnitFloat();
	return GetPointA() * (1 - r1) + GetPointB() * r1 * (1 - r2) + GetPointC() * r1 * r2;
}

Triangle::Triangle(Mesh* mesh, int vertexA, int vertexB, int vertexC) 
	: mesh(mesh) , index(-1), a(vertexA), b(vertexB), c(vertexC)
{ 
	worldToUv = ComputeWorldToUvMatrix();
}

Vector3 Triangle::WorldToUv(Vector3 vec) const
{
	return Math::Convert(worldToUv * Math::Convert(vec - GetPointA())) + GetTexCoordA();
}

Matrix3x3 Triangle::ComputeWorldToUvMatrix() const
{
	/*
		The following computes the transformation matrix from world- to tangent-space.
		I also wrote down the Mathematica input I used for computation:

			T := {-((d n - a p)/(-n o + m p)), -((e n - b p)/(-n o + m p)), -((f n - c p)/(-n o + m p))};
			B := {-((-d m + a o)/(-n o + m p)), -((-e m + b o)/(-n o + m p)), -((-f m + c o)/(-n o + m p))};
			N := Cross[T,B];
			T[a_, b_, c_, d_, e_, f_, m_, n_, o_, p_] := {T, B, N};
			FullSimplify[Inverse[T[a, b, c, d, e, f, m, n, o, p]]]

		The computation should be numerically as stable as possible (at least without
		diving deep into the subject), since it does as little computation as possible
		and only does divisions once per matrix entry.

		The matrix is not in projective space and only works when the triangle and UV space
		are moved into the origin, that is point A of the triangle and point {0,0} in UV space.
		Then you can use this matrix to transform world- to UV-space and add the UV coordinate
		of point A of the triangle to the outcome to move it back into the real UV space of
		the triangle...

		{a,b,c} : World vector (B-A) of the triangle
		{d,e,f} : World vector (C-A) of the triangle
		{m,n} : UV coordinate of B minus UV coordinate of A
		{o,p} : UV coordinate of C minus UV coordinate of A

		The C++ Compiler will compress the computation further by introducing temp variables
		for denominators and squares for instance.
	*/
	Vector3 delta_ba = GetPointB() - GetPointA();
	Vector3 delta_ca = GetPointC() - GetPointA();
	Vector3 uvDelta_ba = GetTexCoordB() - GetTexCoordA();
	Vector3 uvDelta_ca = GetTexCoordC() - GetTexCoordA();

	float 
		a = delta_ba.x,
		b = delta_ba.y,
		c = delta_ba.z,
		d = delta_ca.x,
		e = delta_ca.y,
		f = delta_ca.z,
		m = uvDelta_ba.x,
		n = uvDelta_ba.y,
		o = uvDelta_ca.x,
		p = uvDelta_ca.y;

	return Matrix3x3(
		Math::Convert(Vector3(
				(a * (e*e + f*f) * m + b*b * d * o - a * c * f * o - b * e * (d * m + a * o) + c * d * (-f * m + c * o))
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
			, 
				(a * (e*e + f*f) * n + b*b * d * p - a * c * f * p - b * e * (d * n + a * p) + c * d * (-f * n + c * p))
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
			, 
				((c * e - b * f) * (n * o - m * p))
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
		)), 
		Math::Convert(Vector3(
				(-e * (a * d + c * f) * m + b * (d*d + f*f) * m + (a*a + c*c) * e * o - b * (a * d + c * f) * o)
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
			, 
				(-e * (a * d + c * f) * n + b * (d*d + f*f) * n + (a*a + c*c) * e * p - b * (a * d + c * f) * p)
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
			, 
				-(((c * d - a * f) * (n * o - m * p))
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f)))
		)), 
		Math::Convert(Vector3(
				(c * (d*d + e*e) * m - (a * d + b * e) * f * m - c * (a * d + b * e) * o + (a*a + b*b) * f * o)
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
			, 
				(c * (d*d + e*e) * n - (a * d + b * e) * f * n - c * (a * d + b * e) * p + (a*a + b*b) * f * p)
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))
			, 
				((b * d - a * e) * (n * o - m * p))
				/
				(c*c * (d*d + e*e) - 2 * a * c * d * f - 2 * b * e * (a * d + c * f) + b*b * (d*d + f*f) + a*a * (e*e + f*f))))
	);
 }

float GetTriangleArea(Vector3 a, Vector3 b, Vector3 c)
{
	return 0.5f * Math::Length(Math::Cross(c - a, c - b));
}

Vector3 Triangle::GetNormal(const Vector3& where) const
{
	// barycentric interpolation based on geometry data
	const Vector3 x1 = GetPointA(), x2 = GetPointB(), x3 = GetPointC(), p = where;
	const float A1 = GetTriangleArea(x2, p, x3), A2 = GetTriangleArea(x3, p, x1), A3 = GetTriangleArea(x1, p, x2);

	Vector3 N = Math::Normalized(
		(A1 * mesh->vertices[a].normal + 
		A2 * mesh->vertices[b].normal + 
		A3 * mesh->vertices[c].normal) / GetArea());

	// apply optional bump mapping
	auto material = GetMesh()->GetMaterial();

	if(material->GetBumpTexture() && (material->GetName().compare("vcWatr") == 0))
	{
		// perturbate normal
		const TextureMap& bumpMap = *material->GetBumpTexture();
		Pixel texNp = bumpMap.Interpolated(*this, where);
		Vector3 texN = (Vector3(texNp.r * 2 - 1, texNp.g * 2 - 1, texNp.b * 2 - 1) + Vector3(0,0,1)) / 2;

		auto rot = Vectormath::Aos::Quat::rotation(Vectormath::Aos::Vector3(0,0,1), Math::Convert(N));
		N = Math::Convert(Vectormath::Aos::rotate(rot, Math::Convert(texN)));
	}

	return N;
}

Vector3 Triangle::GetNormal(const PathSegment& incoming) const
{
	Vector3 normal = GetNormal(incoming.GetImpact());

	// Flip normal if on opposite site of impact!

	if((normal ^ incoming.GetDirection()) > 0)
		return - normal;
	else
		return normal;
}