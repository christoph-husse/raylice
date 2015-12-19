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

void RayTracer::ComputeDirectIllumination_Cluster(ThreadContext& ctx, const MSAACluster& cluster) const
{
	int iEntry = 0;

	for(const PathSegment* directPhoton : cluster.photons)
	{
		auto bsdfEntry = cluster.entries[iEntry++ % cluster.entries.size()];
		PathSegment view = bsdfEntry->view;

		// calculate direction vector towards light source
		const PathSegment* emitted = directPhoton->GetSource();

		assert(emitted != nullptr);

		auto light = emitted->GetLight();
		Vector3 dir;

		if(light->IsDirectional())
			dir = -emitted->GetDirection();
		else
			dir = emitted->GetOrigin() - view.GetImpact();

		dir = Math::Normalized(dir);

		// traverse on a straight line towards light source, skipping transparent BSDFs
		BSDFMaterial* lightMaterial = emitted->GetMaterialAtOrigin();
		PathSegment shadowSegment;

		shadowSegment.SetOrigin(view.GetImpact());
		shadowSegment.SetDirection(dir);
		shadowSegment.SetTriangleAtOrigin(view.GetTriangleAtImpact());

		// TODO: "false" workaround as long as there are no caustics, otherwise our CourtYard pool will be black :(
		switch(ctx.FollowTransmissiveEx(shadowSegment, nullptr, false/*light->IsDirectional()*/))
		{
		case ETransmissionResult::EmptySpace:
			bsdfEntry->color = ctx.GetTracer()->GetClearColor();
			break;

		case ETransmissionResult::NotAlive:
		case ETransmissionResult::Dispersed:
			bsdfEntry->color = Pixel();
			break;
		
		case ETransmissionResult::Success:
			// path leads to non-transmissive material
			BSDFMaterial* impactMat = shadowSegment.GetMaterialAtImpact();
			PathSegment lightSegment;

			lightSegment.SetDirection(-dir);
			lightSegment.SetOrigin(view.GetImpact() + dir);
			lightSegment.SetImpact(view.GetImpact());
			lightSegment.SetTriangleAtImpact(view.GetTriangleAtImpact());
			lightSegment.SetTriangleAtOrigin(shadowSegment.GetTriangleAtImpact());

			if(impactMat != lightMaterial)
			{
				// this path does not lead to a light-source!
				lightSegment.SetColor(Pixel());
			}
			else
			{
				// we reached the light source!
				lightSegment.SetColor(emitted->GetColor());
			}

			bsdfEntry->color = bsdfEntry->bsdf->ComputeDirectIllumination(ctx, view, lightSegment);

			if(!bsdfEntry->material->HasCausticBsdfs())
				bsdfEntry->color *= bsdfEntry->material->ShadeSurface(view);

			break;
		}
	}
}

WeightedPixel RayTracer::ComputeDirectIllumination_MSAA(ThreadContext& ctx, const std::vector<PathSegment>& msaaView) const
{
	// Try to cover all MSAA samples with minimal amount of nearest neighbor queries...
	// May not look like much but this gives a speedup with a factor in O(MSAASamples).
	ctx.ResetMSAAClusters();
	ctx.ResetBSDFGroups();

	int iCluster = 0;
	for(const auto& view : msaaView)
	{
		bool wasHandled = false;
		
		// track transmissive BSDFs until we hit a non-transmissive one or empty space or our view dies...
		auto bsdfGroup = ctx.AllocateBsdfGroup();
		BSDFMaterial::TrackCameraPath(ctx, view, bsdfGroup.get());

		for(const auto& bsdfEntry : *bsdfGroup)
		{
			// we hit a non-transmissive BSDF
			if(!bsdfEntry->bsdf)
				continue;

			if(bsdfEntry->bsdf->IsLightingBsdf())
			{
				// try to use existing cluster for view
				bool hasCluster = false;

				for(auto& cluster : ctx.msaaClusters)
				{
					if(!cluster.IsInitialized())
						break;

					if(cluster.Add(bsdfEntry.get()))
					{
						hasCluster = true;
						break;
					}
				}

				if(hasCluster)
					continue;

				// create new cluster (expensive)
				ctx.GetDirectMap().Sample(bsdfEntry->view.GetImpact(), 256, ctx.samples);

				if(ctx.msaaClusters.size() <= iCluster)
					ctx.msaaClusters.push_back(MSAACluster());

				MSAACluster& cluster = ctx.msaaClusters[iCluster++];
				cluster.Initialize(bsdfEntry.get(), ctx.samples);
			}
			else if(bsdfEntry->bsdf->IsCausticBsdf())
			{
				bsdfEntry->color = bsdfEntry->material->ShadeSurface(bsdfEntry->view);
			}
		}
	}

	// go through all clusters seperately and compute resulting illumination for each BSDF entry
	for(const auto& cluster : ctx.msaaClusters)
	{
		if(!cluster.IsInitialized())
			break;

		ComputeDirectIllumination_Cluster(ctx, cluster);
	}

	// evaluate all bsdf groups and average over all groups for final color
	WeightedPixel result(0);

	for(const auto& bsdfGroup : ctx.bsdfGroups)
	{
		Pixel color = Pixel(1,1,1);

		for(const auto& bsdfEntry : *bsdfGroup)
		{
			color *= bsdfEntry->color;
		}

		result += WeightedPixel(1, color);
	}

	return WeightedPixel(result.weight, result.color);
}