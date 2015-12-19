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


#include "BSDFMaterial.h"
#include "Mesh.h"

BSDFMaterial* PathSegment::GetMaterialAtImpact() const { return dstTriangle->GetMesh()->GetMaterial(); }
BSDFMaterial* PathSegment::GetMaterialAtOrigin() const { return srcTriangle->GetMesh()->GetMaterial(); }
bool PathSegment::ImpactOnBackface() const { return GetTriangleAtImpact()->HasHitBackface(GetDirection()); }
bool PathSegment::ImpactOnFrontface() const { return !ImpactOnBackface(); }
const LightSource* PathSegment::GetLight() const { return GetSource() ? GetSource()->GetTriangleAtOrigin()->GetMesh()->GetLight() : nullptr; }

BSDFMaterial::BSDFMaterial() : name(), settings(RenderSettings::Empty()), mTemplate()
{
}

std::shared_ptr<BSDFMaterial> BSDFMaterial::FromTemplate(std::shared_ptr<UnifiedSettings> mTemplate)
{
	std::shared_ptr<BSDFMaterial> mat = std::make_shared<BSDFMaterial>();

	mat->mTemplate = mTemplate;
	mat->name = mTemplate->GetString("Name");

	auto reflective = ReflectiveBSDF::TryFromTemplate(mat);
	auto refractive = RefractiveBSDF::TryFromTemplate(mat);
	auto transparent = TransmissiveBSDF::TryFromTemplate(mat);
	auto lambert = LambertBSDF::TryFromTemplate(mat);
	auto diffuse = DiffuseBSDF::TryFromTemplate(mat);

	//validate material
	if(mat->bouncingBsdfs.size() == 0)
	{
		std::cout << std::endl << "[WARNING]: Material \"" << mat->name << "\" is invalid." << std::endl;
		mat->MakeInvalid();
	}

	return mat;
}

void BSDFMaterial::MakeInvalid()
{
	causticBsdfs.clear();
	surfaceBsdfs.clear();
	lightingBsdfs.clear();

	AddBSDF<LambertBSDF>();
	AddBSDF<DiffuseBSDF>();
}

void BSDFMaterial::ApplyDefaultSettings(const RenderSettings& defaults)
{
	settings.ApplyDefaults(defaults);

	for(auto& bsdf : causticBsdfs) bsdf->ApplyDefaultSettings(settings);
	for(auto& bsdf : surfaceBsdfs) bsdf->ApplyDefaultSettings(settings);
	for(auto& bsdf : lightingBsdfs) bsdf->ApplyDefaultSettings(settings);
}

Pixel BSDFMaterial::ShadeSurface(const PathSegment& view) const
{
	if(surfaceBsdfs.empty())
		return Pixel(1,1,1);
	else
		return ((SurfaceBSDFBase*)SelectBSDF(surfaceBsdfs, view))->ShadeSurface(view);
}

void BSDFMaterial::TrackCameraPath(ThreadContext& ctx, PathSegment view, std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>* outPath)
{
	const PathSegment originalLine = view;
	bool shouldContinue = true;

	while(shouldContinue)
	{
		shouldContinue = false;

		if(!view.CanBounceAgain())
		{
			// view died
			break;
		}

		MultiplicativeBSDFEntry bsdfEntry;

		if(!view.HasImpact() && !ctx.CastRay(view, view))
		{
			// we hit empty space
			bsdfEntry.color = ctx.GetClearColor();
			bsdfEntry.view = view;

			outPath->push_back(ctx.AllocateBsdfEntry(bsdfEntry));

			break;
		}

		auto impactMat = view.GetMaterialAtImpact();
		float probCaustic = 1;
		float probLighting = 1;

		bsdfEntry.view = view;
		bsdfEntry.material = impactMat;

		if(!impactMat->lightingBsdfs.empty() && !impactMat->causticBsdfs.empty())
		{
			probCaustic = 0;
			probLighting = 0;

			for(int i = 0; i < impactMat->causticBsdfs.size(); i++) probCaustic += impactMat->causticBsdfs[i]->GetVisibility()->Get(view);
			for(int i = 0; i < impactMat->lightingBsdfs.size(); i++) probLighting += impactMat->lightingBsdfs[i]->GetVisibility()->Get(view);
		}
		
		if(!impactMat->causticBsdfs.empty())
		{
			auto causticBsdf = impactMat->SelectBSDF(impactMat->causticBsdfs, view);
			PathSegment next;

			bsdfEntry.probability = probCaustic;
			bsdfEntry.bsdf = causticBsdf;
			outPath->push_back(ctx.AllocateBsdfEntry(bsdfEntry));

			if(causticBsdf->TransmitCamera(ctx, view, &next))
			{
				shouldContinue = true;

				view.Bounce();
				view.SetOrigin(view.GetImpact());
				view.SetTriangleAtOrigin(view.GetTriangleAtImpact());
				view.SetTriangleAtImpact(nullptr);
				view.SetDirection(next.GetDirection());
			}
		}

		if(!impactMat->lightingBsdfs.empty())
		{
			bsdfEntry.probability = probLighting;
			bsdfEntry.bsdf = impactMat->SelectBSDF(impactMat->lightingBsdfs, view);
			outPath->push_back(ctx.AllocateBsdfEntry(bsdfEntry));
		}
	}
}

void BSDFMaterial::Transmit(ThreadContext& ctx, const PathSegment& incoming, std::vector<std::pair<PathSegment, PathSegment>>& transmissions) const
{
	for(int i = 0; i < bouncingBsdfs.size(); i++)
	{
		PathSegment copy(incoming);
		const int bounceCount = copy.GetBounceCount();
		PathSegment out = bouncingBsdfs[i]->Transmit(ctx, copy);

		assert(bounceCount == out.GetBounceCount() - 1);

		if(out.IsAlive())
			transmissions.push_back(std::make_pair(copy, out));
	}
}


BSDF* BSDFMaterial::SelectBSDF(const PathSegment& view) const
{
	float totalBsdfProbability = 0;
	for(int i = 0; i < bouncingBsdfs.size(); i++)
	{
		totalBsdfProbability += bouncingBsdfs[i]->GetVisibility()->Get(view);
	}

	float probValue = totalBsdfProbability * Math::GetRandomUnitFloat();
	int i = 0;
	for(; i < bouncingBsdfs.size() - 1; i++)
	{
		auto bsdf = bouncingBsdfs[i].get();
		probValue -= bsdf->GetVisibility()->Get(view);

		if(probValue < 0) 
			break;
	}
	 
	return bouncingBsdfs[Math::Clamp(i, 0, (int)bouncingBsdfs.size() - 1)].get();
}

BSDF* BSDFMaterial::TransmitCamera(ThreadContext& ctx, const PathSegment& view, PathSegment* next) const
{
	auto bsdf = SelectBSDF(causticBsdfs, view);
	
	if(!bsdf->TransmitCamera(ctx, view, next))
	{
		*next = PathSegment();
		return bsdf;
	}
	else
	{
		return nullptr;
	}
}

int BSDFMaterial::GetShadowSampleCount(ThreadContext& ctx, const PathSegment& view) const
{ 
	return (int)std::max(1, ctx.GetSettings().shadowSamples); 
}
