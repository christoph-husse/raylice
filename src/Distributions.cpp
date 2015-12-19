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


float CosineDistribution::GetDistribution(const PathSegment& viewer, const PathSegment& incomingLight) const 
{
	return Math::Saturate(-(incomingLight.GetDirection() ^ incomingLight.GetNormalAtImpact()));
}

Vector3 CosineDistribution::GetHemisphereSample(const PathSegment& viewer) const 
{
	// importance sampling from hemisphere (basically favors direction yielding high values
	// for cosine, since this is the illumination factor for lambertian materials). Will
	// make indirect illumination converge much faster (less noise at low quality settings).

	// Source: http://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/
	const Vector3 n = viewer.GetNormalAtImpact();

	float Xi1 = Math::GetRandomUnitFloat();
	float Xi2 = Math::GetRandomUnitFloat();

	float  theta = acos(sqrt(1.0f - Xi1));
	float  phi = 2.0f * Math::PI * Xi2;

	float xs = sinf(theta) * cosf(phi);
	float ys = cosf(theta);
	float zs = sinf(theta) * sinf(phi);

	Vector3 y(n);
	Vector3 h = y;
	if ((fabs(h.x) <= fabs(h.y)) && (fabs(h.x) <= fabs(h.z)))
		h.x= 1.0;
	else if ((fabs(h.y) <= fabs(h.x)) && (fabs(h.y) <= fabs(h.z)))
		h.y= 1.0;
	else
		h.z= 1.0;

	Vector3 x = Math::Normalized(Math::Cross(h, y));
	Vector3 z = Math::Normalized(Math::Cross(x, y));

	return Math::Normalized(xs * x + ys * y + zs * z);
}

Vector3 PowerCosineDistribution::GetHalfVector(const PathSegment& viewer, const PathSegment& incomingLight)
{
	auto wo = -viewer.GetDirection();
	auto wi = -incomingLight.GetDirection();
	float len = Math::Length(wo + wi);

	if((len != len) || (len < 0.001f))
		return Math::Normalized(wo);
	else
		return Math::Normalized(wi + wo);
}

float PowerCosineDistribution::GetDistribution(const PathSegment& viewer, const PathSegment& incomingLight) const 
{
	const Vector3 halfVector = GetHalfVector(viewer, incomingLight);
	const Vector3 upVector = viewer.GetNormalAtImpact();

	const float cosTheta = (halfVector ^ upVector);
	return norm2 * std::pow(std::abs(cosTheta), glossy);
}

Vector3 PowerCosineDistribution::GetHemisphereSample(const PathSegment& viewer) const 
{
	// http://cg.informatik.uni-freiburg.de/course_notes/graphics2_04_sampling.pdf
	Vector3 disc = Math::AbsPerElem(Math::GetRandomVectorInUnitDisc());
	const Vector3 w = viewer.GetNormalAtImpact();
	const Vector3 u = Math::GetOrthogonal(w);
	const Vector3 v = Math::Cross(w, u);
	const float phi = 2 * Math::PI * disc.x;
	const float theta = std::acosf(std::powf(1 - disc.y, 1 / (glossy + 1)));

	return std::sinf(theta) * std::cosf(phi) * u + std::sinf(phi) * std::sinf(theta) * v + std::cosf(theta) * w;
}