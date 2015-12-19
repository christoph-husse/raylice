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


LambertBSDF::LambertBSDF() 
	: 
	emissiveIntensity(0), 
	isEmissive(false)
{
	SetDistribution(std::make_shared<CosineDistribution>());
}

LambertBSDF* LambertBSDF::TryFromTemplate(std::shared_ptr<BSDFMaterial> material)
{
	auto mTemplate = material->GetTemplate();
	auto lambert = mTemplate->TryGetInstancePath("Lighting", "Lambert");

	if(!lambert)
		return nullptr;

	auto res = material->AddBSDF<LambertBSDF>();

	res->SetEmissiveIntensity(lambert->GetSingle("EmissiveIntensity"));
	res->SetEmissive(res->GetEmissiveIntensity() > 0.0001);

	return res;
}

PathSegment LambertBSDF::Transmit(ThreadContext& ctx, PathSegment& incoming) const
{
	incoming.SetWeight(incoming.GetWeight() * 0.5f);

	PathSegment segment = incoming;

	segment.Bounce();
	segment.SetOrigin(incoming.GetImpact());
	segment.SetDirection(Math::GetRandomVectorInUnitHalfSphere(incoming.GetNormalAtImpact()));
	segment.ClearSource();

	return segment;
}

Pixel LambertBSDF::ComputeDirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const
{
	float bsdf = std::max(0.0, 1.0 / Math::PI * GetDistribution(viewer, incomingLight));

	if(isEmissive)
		return Math::MaxPerElem(bsdf * incomingLight.GetColor(), emissiveIntensity * Pixel(1,1,1));
	else
		return bsdf * incomingLight.GetColor();
}

Pixel LambertBSDF::ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const
{
	if(isEmissive)
		return Math::MaxPerElem(incomingLight.GetColor(), emissiveIntensity * Pixel(1,1,1)) * incomingLight.GetWeight();
	else
		return incomingLight.GetWeight() * incomingLight.GetColor();
}