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

struct FresnelTerm
{
	float n1, n2; 
	Vector3 normal;
	Vector3 reflectDir;
	Vector3 refractDir;
	float refractWeight;
	float reflectWeight;

	FresnelTerm(const PathSegment& incoming, float refractionIndex)
	{
		normal = incoming.GetNormalAtImpact();

		if(incoming.ImpactOnFrontface())
		{
			// from outside the object
			n1 = 1;
			n2 = refractionIndex;
		}
		else
		{
			// from inside the object
			n1 = refractionIndex;
			n2 = 1;
		}

		float eta = n1/n2;
		float cosI = -(incoming.GetDirection() ^ normal);
		float sinT = eta*eta*(1 - cosI*cosI);

		reflectWeight = 1;
		reflectDir = incoming.GetDirection() + 2*cosI*normal;

		if(sinT < 1)
		{
			float cosT = std::sqrtf(1 - sinT);
			refractDir = eta*incoming.GetDirection() + (eta*cosI - cosT) * normal;

			float ROi = (n1*cosI - n2*cosT) / (n1*cosI + n2*cosT);
			float RPi = (n2*cosI - n1*cosT) / (n2*cosI + n1*cosT);

			ROi *= ROi;
			RPi *= RPi;

			reflectWeight = std::min(1.0f, (ROi + RPi) / 2);
		}

		refractWeight = 1 - reflectWeight;
	}
};


float FresnelRefractance::Get(const PathSegment& incoming) const
{
	FresnelTerm fresnel(incoming, refractiveIndex);
	return fresnel.refractWeight * BSDFVisibility::Get(incoming);
}

float FresnelReflectance::Get(const PathSegment& incoming) const
{
	FresnelTerm fresnel(incoming, refractiveIndex);
	return fresnel.reflectWeight * BSDFVisibility::Get(incoming);
}

void ReflectiveOrRefractiveBSDF::FromTemplate(std::shared_ptr<BSDFMaterial> material, std::shared_ptr<UnifiedSettings> caustic)
{
	auto mTemplate = material->GetTemplate();
	auto fresnel = caustic->TryGetInstancePath("Visibility", "Fresnel");

	SetGlossyAngleRadians(caustic->GetSingle("GlossyAngle"));

	if(fresnel)
		SetRefractiveIndex(fresnel->GetVector3("RealRefractiveIndex").x);
	else
		SetRefractiveIndex(1.5);
}

ReflectiveBSDF* ReflectiveBSDF::TryFromTemplate(std::shared_ptr<BSDFMaterial> material)
{
	auto mTemplate = material->GetTemplate();
	auto reflection = mTemplate->TryGetInstancePath("Caustic", "Reflection");
	if(!reflection)
		return nullptr;

	auto res = material->AddBSDF<ReflectiveBSDF>();
	res->FromTemplate(material, reflection);
	return res;
}

RefractiveBSDF* RefractiveBSDF::TryFromTemplate(std::shared_ptr<BSDFMaterial> material)
{
	auto mTemplate = material->GetTemplate();
	auto refraction = mTemplate->TryGetInstancePath("Caustic", "Refraction");
	if(!refraction)
		return nullptr;

	auto res = material->AddBSDF<RefractiveBSDF>();
	res->FromTemplate(material, refraction);
	return res;
}

PathSegment ReflectiveBSDF::Transmit(ThreadContext& ctx, PathSegment& incoming) const
{
	FresnelTerm fresnel(incoming, GetRefractiveIndex());
	PathSegment segment = incoming;
	float weight = GetVisibility()->Get(incoming);

	segment.Bounce();
	segment.SetOrigin(incoming.GetImpact());
	segment.SetDirection(Math::GetRandomVectorInSolidAngle(fresnel.reflectDir, GetGlossyAngle()));

	return segment;
}

PathSegment RefractiveBSDF::Transmit(ThreadContext& ctx, PathSegment& incoming) const
{
	FresnelTerm fresnel(incoming, GetRefractiveIndex());
	PathSegment segment = incoming;
	float weight = GetVisibility()->Get(incoming);
	
	segment.Bounce();
	segment.SetOrigin(incoming.GetImpact());
	segment.SetDirection(Math::GetRandomVectorInSolidAngle(fresnel.refractDir, GetGlossyAngle()));

	return segment;
}