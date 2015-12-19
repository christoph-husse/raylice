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


CookTorranceBSDF::CookTorranceBSDF()
{
}

PathSegment CookTorranceBSDF::Transmit(ThreadContext& ctx, PathSegment& incoming) const
{
	incoming.SetWeight(incoming.GetWeight() * 0.5f);

	PathSegment segment = incoming;

	segment.Bounce();
	segment.SetOrigin(incoming.GetImpact());
	segment.SetDirection(Math::GetRandomVectorInUnitHalfSphere(incoming.GetNormalAtImpact()));
	segment.ClearSource();

	return segment;
}

Pixel CookTorranceBSDF::ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const
{
	return Pixel(1,1,1);
}

Pixel CookTorranceBSDF::ComputeDirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const
{
	return Pixel(1,1,1);
}
