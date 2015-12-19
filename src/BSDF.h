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


#include "RenderSettings.h"
#include "TextureMap.h"

#include <vector>
#include <memory>

class Distribution;

enum class KnownBSDF
{
	Lambert,
	Transmissive,
	Reflective,
	Refractive,
	Microfacet
};


enum class BSDFVisibilityTerm
{
	Default,
    FresnelReflectance,
    FresnelRefractance,
};

class BSDFVisibility
{
private:
	float percentage;
	std::shared_ptr<TextureMap> alphaMap;
	bool isAlphaMapInverted;
public:

	BSDFVisibility() : percentage(1), isAlphaMapInverted(false) {}
	virtual ~BSDFVisibility() { }

	void SetPercentage(float value) { percentage = Math::Clamp(value, 0.01f, 100.0f); }
	float GetPercentage() const { return percentage; }
	void SetAlphaMap(std::shared_ptr<TextureMap> texture, bool isInverted) { alphaMap = texture; isAlphaMapInverted = isInverted; }
	const TextureMap* GetAlphaMap() const { return alphaMap.get(); }
	bool IsAlphaMapInverted() const { return isAlphaMapInverted; }

	virtual float Get(const PathSegment& incoming) const 
	{
		if(alphaMap)
		{
			Pixel alphaPixel = (*alphaMap)(*incoming.GetTriangleAtImpact(), incoming.GetImpact());
			float alpha = alphaPixel.a;

			if(alphaMap->GetChannelCount() == 3)
				alpha = alphaPixel.r;

			return percentage * (isAlphaMapInverted ? 1 - alpha : alpha); 
		}
		else
			return percentage; 
	}
};

enum class EBSDFKind
{
	Caustic,
	Surface,
	Lighting,
};

class BSDF
{
private:

	std::shared_ptr<BSDFVisibility> visibility;
	std::shared_ptr<Distribution> distribution;
	RenderSettings settings;
	EBSDFKind kind;
	BSDFMaterial* material;
public: 

	virtual ~BSDF() { }
	BSDF(EBSDFKind kind) : settings(RenderSettings::Empty()), visibility(std::make_shared<BSDFVisibility>()), kind(kind) { }

	void SetMaterial(BSDFMaterial* material) { this->material = material; }
	BSDFMaterial* GetMaterial() { return this->material; }

	EBSDFKind GetKind() const { return kind; }
	bool IsLightingBsdf() const { return kind == EBSDFKind::Lighting; }
	bool IsCausticBsdf() const { return kind == EBSDFKind::Caustic; }
	bool IsSurfaceBsdf() const { return kind == EBSDFKind::Surface; }
	const BSDFVisibility* GetVisibility() const { return visibility.get(); }
	BSDFVisibility* GetVisibility() { return visibility.get(); }

	void SetVisibility(std::shared_ptr<BSDFVisibility> copied) { visibility = copied; }

    float GetDistribution(const PathSegment& viewer, const PathSegment& incomingLight) const;// { return distribution->GetDistribution(viewer, incomingLight); }
    Vector3 GetHemisphereSample(const PathSegment& viewer) const;// { return distribution->GetHemisphereSample(viewer); }
	void SetDistribution(std::shared_ptr<Distribution> value) { distribution = value; }

	virtual PathSegment Transmit(ThreadContext& ctx, PathSegment& incoming) const = 0;
	virtual bool TransmitCamera(ThreadContext& ctx, const PathSegment& view, PathSegment* next) const
	{
		PathSegment copy(view); // camera view shall not be affected by tranmissions!
		*next = Transmit(ctx, copy);
		return true;
	}

	const RenderSettings& GetSettings() const { return settings; }
	void SetSettings(const RenderSettings& settings) { this->settings = settings; }
	void ApplyDefaultSettings(const RenderSettings& defaults) { settings.ApplyDefaults(defaults); }


	virtual Pixel ComputeDirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const = 0;
	virtual Pixel ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const = 0;

	Pixel ComputeLocalIllumination(ThreadContext& ctx, const PathSegment& viewer) const;
	Pixel ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& view) const;
};

class CausticBSDFBase : public BSDF
{
public:
	CausticBSDFBase() : BSDF(EBSDFKind::Caustic) { }
};

class SurfaceBSDFBase : public BSDF
{
public:
	SurfaceBSDFBase() : BSDF(EBSDFKind::Surface) { }

	virtual Pixel ShadeSurface(const PathSegment& viewer) const = 0;
};


class LightingBSDFBase : public BSDF
{
public:
	LightingBSDFBase() : BSDF(EBSDFKind::Lighting) { }
};