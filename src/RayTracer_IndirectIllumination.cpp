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


Pixel RayTracer::ComputeIndirectIllumination_MSAA(ThreadContext& ctx, const std::vector<PathSegment>& msaaView) const
{
	WeightedPixel pixel;
	for(PathSegment view : msaaView)
	{
		view.ResetImpact();

		ctx.ResetBSDFGroups();
		auto bsdfGroup = ctx.AllocateBsdfGroup();
		BSDFMaterial::TrackCameraPath(ctx, view, bsdfGroup.get());
		Pixel color(1,1,1);

		for(const auto& bsdfEntry : *bsdfGroup)
		{
			bool hasCaustic = false;
			auto bsdf = bsdfEntry->bsdf;

			if(!bsdf)
				continue;

			if(bsdf->IsLightingBsdf())
			{
				color *= bsdf->ComputeIndirectIllumination(ctx, bsdfEntry->view);
			}
			
			if(bsdf->IsCausticBsdf())
			{
				hasCaustic = true;
				color *= bsdf->GetMaterial()->ShadeSurface(bsdfEntry->view);
			}
			
			if(!hasCaustic)
				color *= bsdf->GetMaterial()->ShadeSurface(bsdfEntry->view);
		}

		pixel += WeightedPixel(1, color);
	}

	return (Pixel)pixel;
}

Pixel BSDF::ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& view) const
{
	WeightedPixel color;

	for(int i = 0; i < GetSettings().subSamples; i++)
	{
		// sample hemisphere
		PathSegment segment = view;
		Pixel estimate;

		segment.SetOrigin(view.GetImpact());
		segment.SetTriangleAtOrigin(view.GetTriangleAtImpact());
		segment.SetDirection(Math::GetRandomVectorInUnitHalfSphere(view.GetNormalAtImpact()));//GetHemisphereSample(view));
		segment.SetColor(ctx.GetTracer()->EstimateIndirectIllumination(ctx, segment));
		segment.SetImpact(segment.GetOrigin() + segment.GetDirection());

		PathSegment incomingLight = PathSegment::Inverse(segment);
		//float dist = GetDistribution(view, incomingLight);

		//if(dist < 0.01f)
		//	continue;

		//incomingLight.SetWeight(1 / dist);
		color += WeightedPixel(incomingLight.GetWeight(), ComputeIndirectIllumination(ctx, view, incomingLight));
	}

	Pixel res = (Pixel)color;
	return res;
}

Pixel RayTracer::EstimateIndirectIllumination(ThreadContext& ctx, const PathSegment& viewOriginal) const
{
	BSDF* bsdf;
	Pixel result;
	PathSegment view = viewOriginal;

	if(ctx.FollowTransmissive(view, &result, &bsdf))
	{
		// find nearest photon
		ctx.GetIndirectMap().Sample(view.GetImpact(), GetSettings().indirectSmoothingSamples, ctx.samples);
		PathSegment* photon = ctx.samples.Select();

		// estimate local illumination around photon
		if(!photon->HasLocalIllumination())
		{
			photon->SetLocalIllumination(photon->GetMaterialAtImpact()->ComputeLocalIllumination(ctx, *photon));
		}

		result = photon->GetLocalIllumination();
	}

	return result;
}

Pixel BSDFMaterial::ComputeLocalIllumination(ThreadContext& ctx, const PathSegment& photon) const
{
	// sample all BSDFs with view-independent contributions
	WeightedPixel result;

	for(int i = 0; i < photon.GetMaterialAtImpact()->GetBSDFCount(); i++)
	{
		PathSegment view = photon;
		BSDF* bsdf;

		if(ctx.FollowTransmissive(view, &result, &bsdf))
		{
			ctx.GetIndirectMap().Sample(view.GetImpact(), GetSettings().indirectLocalSamples, ctx.samples);

			result += WeightedPixel(1, bsdf->ComputeLocalIllumination(ctx, view));
		}
	}

	return (Pixel)result;
}

Pixel BSDF::ComputeLocalIllumination(ThreadContext& ctx, const PathSegment& viewer) const
{
	WeightedPixel result;
	float sampleCount = 0;

	for(PathSegment* sample : ctx.samples)
	{
		PathSegment mutatedLight;

		if(!sample->GetPrevSegment())
		{
			// direct light hit - get path from light source to viewer
			mutatedLight.SetOrigin(sample->GetOrigin());
			mutatedLight.SetDirection(Math::Normalized(viewer.GetImpact() - sample->GetOrigin()));
			mutatedLight.SetImpact(viewer.GetImpact());
			mutatedLight.SetTriangleAtImpact(viewer.GetTriangleAtImpact());
			mutatedLight.SetTriangleAtOrigin(sample->GetTriangleAtOrigin());
			mutatedLight.SetColor(sample->GetColor() * GetSettings().indirectLightAmplifier);
			mutatedLight.SetWeight(1);
		}
		else
		{
			// indirect lighting -> path mutation from neighboring predecessor to viewer
			mutatedLight = PathSegment::FromTo(*sample->GetPrevSegment(), viewer);
			mutatedLight.SetWeight(sample->GetWeight());
		}

		mutatedLight.ResetImpact();

		BSDF* bsdf;
		const float weight = mutatedLight.GetWeight();
		Pixel color;

		if(ctx.FollowTransmissive(mutatedLight, &color, &bsdf))
		{
			// is impact near viewer?
			if((mutatedLight.GetMeshAtImpact() != viewer.GetMeshAtImpact()) ||
					(Math::Length(mutatedLight.GetImpact() - viewer.GetImpact()) > GetSettings().indirectLightTolerance))
			{
				continue;
			}
			else
			{
				result += WeightedPixel(1, bsdf->ComputeDirectIllumination(ctx, viewer, mutatedLight) * bsdf->GetMaterial()->ShadeSurface(viewer));
			}
		}
		else
			result += WeightedPixel(1, color);
	}

	return (Pixel)result;
}