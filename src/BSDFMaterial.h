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

#include "BSDF.h"

#include <vector>
#include <memory>

class UnifiedSettings;
class MultiplicativeBSDFEntry;

class BSDFMaterial
{
private:
	std::vector<std::shared_ptr<CausticBSDFBase>> causticBsdfs;
	std::vector<std::shared_ptr<SurfaceBSDFBase>> surfaceBsdfs;
	std::vector<std::shared_ptr<LightingBSDFBase>> lightingBsdfs;
	std::vector<std::shared_ptr<BSDF>> bouncingBsdfs;
	std::string name;
	RenderSettings settings;
	std::shared_ptr<TextureMap> bumpTexture;
	std::shared_ptr<UnifiedSettings> mTemplate;

	void MakeInvalid();

public:
	BSDFMaterial();

	static std::shared_ptr<BSDFMaterial> FromTemplate(std::shared_ptr<UnifiedSettings> mTemplate);

	const std::string& GetName() const { return name; }
	UnifiedSettings* GetTemplate() const { return mTemplate.get(); }

	template<class TBSDF>
	typename std::enable_if<std::is_base_of<CausticBSDFBase, TBSDF>::value, TBSDF*>::type AddBSDF()
	{
		auto bsdf = std::make_shared<TBSDF>();
		bsdf->SetMaterial(this);
		causticBsdfs.emplace_back(bsdf);
		bouncingBsdfs.push_back(bsdf);
		return (TBSDF*)bsdf.get();
	}

	template<class TBSDF>
	typename std::enable_if<std::is_base_of<SurfaceBSDFBase, TBSDF>::value, TBSDF*>::type AddBSDF()
	{
		auto bsdf = std::make_shared<TBSDF>();
		bsdf->SetMaterial(this);
		surfaceBsdfs.emplace_back(bsdf);
		return (TBSDF*)bsdf.get();
	}

	template<class TBSDF>
	typename std::enable_if<std::is_base_of<LightingBSDFBase, TBSDF>::value, TBSDF*>::type AddBSDF()
	{
		auto bsdf = std::make_shared<TBSDF>();
		bsdf->SetMaterial(this);
		lightingBsdfs.emplace_back(bsdf);
		bouncingBsdfs.push_back(bsdf);
		return (TBSDF*)bsdf.get();
	}

	template<class TBSDF>
	void AddBSDF()
	{
        static_assert(std::is_base_of<CausticBSDFBase, TBSDF>::value ||
                      std::is_base_of<SurfaceBSDFBase, TBSDF>::value ||
                      std::is_base_of<LightingBSDFBase, TBSDF>::value,
                      "A custom BSDF shall derive from either CausticBSDFBase, SurfaceBSDFBase or LightingBSDFBase.");
		return nullptr;
	}

	
	static void TrackCameraPath(ThreadContext& ctx, PathSegment view, std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>* outPath);

	int GetBSDFCount() const { return causticBsdfs.size() + surfaceBsdfs.size() + lightingBsdfs.size(); }

	const RenderSettings& GetSettings() const { return settings; }
	void SetSettings(const RenderSettings& settings) { this->settings = settings; }

	void ApplyDefaultSettings(const RenderSettings& defaults);

	template<class TBSDFContainer>
	BSDF* SelectBSDF(TBSDFContainer bsdfs, const PathSegment& view) const
	{
		float totalBsdfProbability = 0;
		for(int i = 0; i < bsdfs.size(); i++)
		{
			totalBsdfProbability += bsdfs[i]->GetVisibility()->Get(view);
		}

		float probValue = totalBsdfProbability * Math::GetRandomUnitFloat();
		int i = 0;
		for(; i < bsdfs.size() - 1; i++)
		{
			auto bsdf = bsdfs[i].get();
			probValue -= bsdf->GetVisibility()->Get(view);

			if(probValue < 0) 
				break;
		}
	 
		return bsdfs[Math::Clamp(i, 0, (int)bsdfs.size() - 1)].get();
	}

	bool HasCausticBsdfs() const { return !causticBsdfs.empty(); }
	Pixel ShadeSurface(const PathSegment& view) const;
	BSDF* SelectBSDF(const PathSegment& view) const;
	void Transmit(ThreadContext& ctx, const PathSegment& incoming, std::vector<std::pair<PathSegment, PathSegment>>& transmissions) const;
	BSDF* TransmitCamera(ThreadContext& ctx, const PathSegment& view, PathSegment* next) const;
	int GetShadowSampleCount(ThreadContext& ctx, const PathSegment& view) const;

	std::shared_ptr<TextureMap> ShareBumpTexture() const { return bumpTexture; }
	const TextureMap* GetBumpTexture() const { return bumpTexture.get(); }
	void SetBumpTexture(std::shared_ptr<TextureMap> value) { bumpTexture = value; }

	Pixel ComputeLocalIllumination(ThreadContext& ctx, const PathSegment& photon) const;
};

