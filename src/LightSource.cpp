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

LightSource::LightSource(std::shared_ptr<Mesh> mesh)
	:
	mesh(mesh),
	maximumProbability(0),
	isDirectional(false),
	color(1,1,1),
	texture(),
	index(-1),
	intensity(1), 
	photonMultiplier(1)
{
	float prob = 0;

	for(auto& tri : mesh->GetTriangles())
	{
		probToTriangle[prob] = &tri;
		prob += tri.GetArea();
	}

	maximumProbability = prob - mesh->GetTriangles().back().GetArea();
}

std::shared_ptr<LightSource> LightSource::TryFromTemplate(std::shared_ptr<UnifiedSettings> settings, std::shared_ptr<Mesh> mesh)
{
	auto photon = settings->TryGetInstancePath("Surface", "Diffuse", "Photon");
	if(!photon)
		return nullptr;

	auto light = std::make_shared<LightSource>(mesh);

	light->SetIntensity(photon->GetSingle("PhotonIntensity"));
	light->SetDirectional(photon->GetBoolean("IsDirectional"));

	return light;
}

bool LightSource::EmitPhoton(ThreadContext& ctx) const
{
	double prob = Math::GetRandomUnitFloat() * maximumProbability;
	int index = probToTriangle.lower_bound(prob)->second->GetFaceIndex();
	const Triangle* triangle = &ctx.GetTracer()->GetTriangles()[index];

	Vector3 origin = triangle->GetRandomPoint();
	Vector3 direction = triangle->GetNormal(origin);

	if(!isDirectional)
		direction = Math::GetRandomVectorInUnitHalfSphere(direction, 0);

	Pixel color = GetColor();
	color *= GetIntensity() * ctx.GetTracer()->GetSettings().photonIntensity;

	// insert segment from light source to first impact into photon maps
	PathSegment emitted;

	emitted.SetDirection(direction);
	emitted.SetOrigin(origin);
	emitted.SetTriangleAtOrigin(triangle);
	emitted.SetTriangleAtImpact(triangle); // will be updated during raycast, for now prevents self-intersection!
	emitted.SetColor(color);

	if(!ctx.CastRay(emitted, emitted))
		return false; // we hit empty space, this photon is not going to do any good...

	PathSegment* allocated = ctx.GetDirectMap().Insert(emitted);
	
	if(allocated)
	{
		allocated->source = allocated;
		ctx.GetIndirectMap().Register(allocated);

		emitted = *allocated;
	}

	ctx.GetTracer()->TracePhoton(ctx, emitted);

	return true;
}