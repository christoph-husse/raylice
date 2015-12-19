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

void ThreadContext::ResetMSAAClusters()
{
	for(auto& cluster : msaaClusters)
	{
		cluster.Reset();
	}
}

void ThreadContext::ResetBSDFGroups()
{
	for(auto& group : bsdfGroups)
	{
		for(auto& entry : *group)
		{
			freeBsdfEntries.push_back(entry);
		}

		group->clear();
		freeBsdfGroups.push_back(group);
	}

	bsdfGroups.clear();
}

std::shared_ptr<MultiplicativeBSDFEntry> ThreadContext::AllocateBsdfEntry(MultiplicativeBSDFEntry init)
{
	std::shared_ptr<MultiplicativeBSDFEntry> res;

	if(freeBsdfEntries.size() == 0)
		res = std::make_shared<MultiplicativeBSDFEntry>();
	else
	{
		res = freeBsdfEntries.back();
		freeBsdfEntries.pop_back();
	}

	*res = init;

	return res;
}

std::shared_ptr<std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>> ThreadContext::AllocateBsdfGroup()
{
	std::shared_ptr<std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>> res;

	if(freeBsdfGroups.size() == 0)
		res = std::make_shared<std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>>();
	else
	{
		res = freeBsdfGroups.back();
		freeBsdfGroups.pop_back();
	}

	bsdfGroups.push_back(res);
	return res;
}

const RenderSettings& ThreadContext::GetSettings() const { return rayTracer->settings; }
PhotonMap& ThreadContext::GetIndirectMap() { return rayTracer->GetIndirectMap(); }
PhotonMap& ThreadContext::GetDirectMap() { return rayTracer->GetDirectMap(); }
PhotonMap& ThreadContext::GetCausticsMap() { return rayTracer->GetCausticsMap(); }
bool ThreadContext::CastRay(const PathSegment& incoming, PathSegment& outgoing) { return intersector->CastRay(*this, incoming, outgoing); }
Pixel ThreadContext::GetClearColor() const { return rayTracer->GetClearColor(); }

ThreadContext::ThreadContext(RayTracer* tracer, int threadIndex) 
		: 
		rayTracer(tracer), 
		intersector(),
		threadIndex(threadIndex)
{ 
}

void ThreadContext::SetIntersector(std::shared_ptr<RayIntersector> intersector)
{ 
	this->intersector = intersector;
}

bool ThreadContext::FollowTransmissive(PathSegment& view, Pixel* addToColor, BSDF** outBsdf)
{
	switch(FollowTransmissiveEx(view, outBsdf))
	{
	case ETransmissionResult::EmptySpace:
		*addToColor += GetClearColor();
		return false;

	case ETransmissionResult::NotAlive:
	case ETransmissionResult::Dispersed:
		*addToColor += Pixel();
		return false;
		
	case ETransmissionResult::Success: 
		return true;
	}

	throw std::bad_exception("This should never happen!");
}

ETransmissionResult ThreadContext::FollowTransmissiveEx(PathSegment& line, BSDF** outBsdf, bool onStraightLine)
{
	const PathSegment originalLine = line;

	if(outBsdf != nullptr)
		*outBsdf = nullptr;

	line.SetColor(Pixel(1,1,1));

	while(true)
	{
		if(!line.CanBounceAgain())
			return ETransmissionResult::NotAlive; 

		if(!line.HasImpact() && !CastRay(line, line))
			return ETransmissionResult::EmptySpace; 

		BSDF* bsdf = line.GetMaterialAtImpact()->SelectBSDF(line);
		PathSegment next;

		if(outBsdf != nullptr)
			*outBsdf = bsdf;

		if(bsdf->TransmitCamera(*this, line, &next))
		{
			line.Bounce();
			line.SetOrigin(line.GetImpact());
			line.SetTriangleAtOrigin(line.GetTriangleAtImpact());
			line.SetTriangleAtImpact(nullptr);

			if(onStraightLine)
			{
				if((next.GetDirection() ^ originalLine.GetDirection()) < 0.999f)
					return ETransmissionResult::Dispersed;

				line.SetDirection(originalLine.GetDirection());
			}
			else
				line.SetDirection(next.GetDirection());
		}
		else
		{
			return ETransmissionResult::Success;
		}
	}
}
