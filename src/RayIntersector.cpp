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

struct RayIntersector_Impl
{
	embree::Ref<embree::Accel> accel;
	embree::Ref<embree::Intersector> intersector;
};


RayIntersector::RayIntersector(RayTracer* tracer) : tracer(tracer)
{
	impl = std::make_shared<RayIntersector_Impl>();

	auto triangles = tracer->GetTriangles();
	int triCount = triangles.size();
	embree::BuildTriangle* embreeTri = (embree::BuildTriangle*)embree::rtcMalloc(triCount * sizeof(embree::BuildTriangle));
	embree::BuildVertex* embreeVert = (embree::BuildVertex*)embree::rtcMalloc(triCount * 3 * sizeof(embree::BuildVertex));
	int i = 0, j = 0;

	for(auto& input : triangles)
	{
		auto a = input.GetPointA();
		auto b = input.GetPointB();
		auto c = input.GetPointC();

		embree::BuildTriangle& tri = embreeTri[i];
		tri.id0 = i++;
		tri.id1 = 0;

		tri.v0 = j;
		embreeVert[j++] = embree::BuildVertex(a.x, a.y, a.z);

		tri.v1 = j;
		embreeVert[j++] = embree::BuildVertex(b.x, b.y, b.z);

		tri.v2 = j;
		embreeVert[j++] = embree::BuildVertex(c.x, c.y, c.z);
	}

	impl->accel = embree::rtcCreateAccel("default", "default", embreeTri, triCount, embreeVert, triCount * 3);
	impl->intersector = impl->accel->queryInterface<embree::Intersector>();
}

bool RayIntersector::CastRay(ThreadContext& ctx, const PathSegment& incoming, PathSegment& outgoing)
{
	/*
		ATTENTION:	incoming & outgoing may reference the same memory block, so make sure
					that this case is handled properly...
	*/
	embree::Hit hit;
	Ray ray(outgoing.GetRay());

	if(incoming.GetTriangleAtImpact())
		ctx.forbiddenTriangles.push_back(incoming.GetTriangleAtImpact()->GetFaceIndex());

	impl->intersector->intersect(ray, ctx.forbiddenTriangles, hit);
	ctx.forbiddenTriangles.clear();
	if(!hit)
		return false;

	outgoing.SetImpact(ray.org + hit.t * ray.dir);
	outgoing.SetTriangleAtOrigin(incoming.GetTriangleAtImpact());
	outgoing.SetTriangleAtImpact(&tracer->GetTriangles()[hit.id0]);

	return true;
}
