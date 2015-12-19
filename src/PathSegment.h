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

#include <boost/optional.hpp>

class ThreadContext;
class BSDFMaterial;
class LightSource;

class PathSegment
{
private:
	friend class PhotonMap;
	friend class LightSource;

	// supposed to be a POD type! don't add anything fancy (with a destructor or whatever) here...
	const PathSegment* prevSegment;
	PathSegment* address;
	int bounceCount;
	ThreadContext* context;
	Vector3 origin;
	Vector3 destination;
	Vector3 direction;
	const Triangle* dstTriangle;
	const Triangle* srcTriangle;
	Pixel color;
	float weight;
	boost::optional<Pixel> localIllumination;
	const PathSegment* source;

public:

	PathSegment() 
		:
		prevSegment(nullptr),
		address(nullptr),
		bounceCount(0),
		context(nullptr),
		origin(0,0,0), 
		destination(0,0,0), 
		direction(0,0,0),
		dstTriangle(nullptr),
		srcTriangle(nullptr),
		color(0,0,0),
		weight(1),
		source(nullptr)
	{ }

	static PathSegment FromTo(const PathSegment& from, const PathSegment& to)
	{
		PathSegment result;

		result.color = from.GetColor();
		result.context = from.context;
		result.destination = to.destination;
		result.origin = from.destination;
		result.direction = Math::Normalized(result.destination - result.origin);
		result.dstTriangle = to.dstTriangle;
		result.srcTriangle = from.dstTriangle;

		return result;
	}

	static PathSegment Inverse(const PathSegment& segment)
	{
		PathSegment result = segment;

		result.destination = segment.origin;
		result.origin = segment.destination;
		result.direction = -segment.direction;
		result.dstTriangle = segment.srcTriangle;
		result.srcTriangle = segment.dstTriangle;

		return result;
	}

	const PathSegment* GetAddress() const { return address; }
	PathSegment* GetAddress() { return address; }
	Ray GetRay() const { return Ray(origin, direction); }
	ThreadContext* GetContext() const { return context; }
	Pixel GetColor() const { return color; }
	Vector3 GetOrigin() const { return origin; }
	Vector3 GetImpact() const { return destination; }
	const Triangle* GetTriangleAtImpact() const { return dstTriangle; }
	const Triangle* GetTriangleAtOrigin() const { return srcTriangle; }
	const Mesh* GetMeshAtImpact() const { return dstTriangle->GetMesh(); }
	const Mesh* GetMeshAtOrigin() const { return srcTriangle->GetMesh(); }
	Vector3 GetDirection() const { return direction; }
	BSDFMaterial* GetMaterialAtImpact() const;
	BSDFMaterial* GetMaterialAtOrigin() const;
	Vector3 GetNormalAtImpact() const { return GetTriangleAtImpact()->GetNormal(*this); }
	Vector3 GetNormalAtOrigin() const { return GetTriangleAtOrigin()->GetNormal(*this); }
	bool IsAlive() const { return CanBounceAgain() && !color.IsBlack(); }
	bool CanBounceAgain() const { return (bounceCount < 10); }
	bool HasImpact() const { return dstTriangle != nullptr; }
	bool HasLocalIllumination() const { return localIllumination.is_initialized(); }
	Pixel GetLocalIllumination() const { assert(HasLocalIllumination()); return localIllumination.get_value_or(Pixel()); }
	void SetLocalIllumination(Pixel value) { localIllumination = value; }
	void ResetLocalIllumination() { localIllumination.reset(); }
	bool ImpactOnBackface() const;
	bool ImpactOnFrontface() const;
	const PathSegment* GetPrevSegment() const { return prevSegment; }
	int GetBounceCount() const { return bounceCount; }
	float GetWeight() const { return weight; }
	const LightSource* GetLight() const;
	const PathSegment* GetSource() const { return source; }

	void ResetImpact() { destination = Math::InvalidVector3(); dstTriangle = nullptr; }
	void SetWeight(float value) { weight = value; }
	void SetImpact(Vector3 value) { destination = value; }
	void SetTriangleAtImpact(const Triangle* value) { dstTriangle = value; }
	void SetTriangleAtOrigin(const Triangle* value) { srcTriangle = value; }
	void SetPrevSegment(const PathSegment* segment) { prevSegment = segment; }
	void ClearSource() { source = nullptr; }
	void SetBounceCount(int value) { bounceCount = value; }
	void SetOrigin(Vector3 org) { origin = org; ResetImpact(); }
	void SetDirection(Vector3 dir) { direction = Math::Normalized(dir); ResetImpact(); }
	void SetColor(Pixel color) { this->color = color; }
	void Bounce() { bounceCount = std::max(bounceCount, bounceCount + 1); }
	void Kill() { bounceCount = 0x7FFFFFFF; }
};
