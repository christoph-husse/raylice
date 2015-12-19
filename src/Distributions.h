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



class Distribution
{
public:
	virtual float GetDistribution(const PathSegment& viewer, const PathSegment& incomingLight) const = 0;
	virtual Vector3 GetHemisphereSample(const PathSegment& viewer) const = 0;
};

class CosineDistribution : public Distribution
{
public:

	CosineDistribution() { }

	float GetDistribution(const PathSegment& viewer, const PathSegment& incomingLight) const override;
	Vector3 GetHemisphereSample(const PathSegment& viewer) const override;
};

class PowerCosineDistribution : public Distribution
{
private:
	float glossy; //!< Glossiness with range [0,infinity[ where 0 is a diffuse surface.
	Vector3 upVector; //!< z-direction of the distribution.
	float norm2; // Normalization constant for calculating the distribution.
	float norm1; // Normalization constant for calculating the pdf for sampling.
public:

	PowerCosineDistribution(float glossy)
		: 
		glossy(glossy), 
		norm1((glossy + 1) * (0.5f * Math::PI)), 
		norm2((glossy + 2) * (0.5f * Math::PI)) { }

	static Vector3 GetHalfVector(const PathSegment& viewer, const PathSegment& incomingLight);

	float GetDistribution(const PathSegment& viewer, const PathSegment& incomingLight) const override;
	Vector3 GetHemisphereSample(const PathSegment& viewer) const override;
};
