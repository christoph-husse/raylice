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


#ifndef _RAYTRACERIMPL_H_
#define _RAYTRACERIMPL_H_

#include "RayTracer.h"

class RayIntersector
{
private:
	std::shared_ptr<struct RayIntersector_Impl> impl;
	RayTracer* tracer;
public:
	RayIntersector(RayTracer* tracer);
	bool CastRay(ThreadContext& ctx, const PathSegment& incoming, PathSegment& outgoing);
};

struct ScreenSpacePosition
{
	float xNdc;
	float yNdc;
	float xDelta;
	float yDelta;
	int xScreen;
	int yScreen;
	Ray ray;
	std::function<Ray (float, float)> GetRay;
};

class RayTracer : boost::noncopyable
{
private:
	friend class ThreadContext;
	friend class PhotonMap;
	friend class LightSource;
	friend class Photon;

	RenderSettings settings;
	Camera camera;
	std::shared_ptr<Scene> scene;
	const int width, height;
	PhotonMap indirectMap;
	PhotonMap causticsMap;
	PhotonMap directMap;
	RenderBuffer frameBuffer;

	std::vector<Triangle> triangles;
	std::vector<BSDFMaterial*> triToMatMap;
	std::vector<ThreadContext> threadCtx;

	void RunParallel(std::function<void (ThreadContext& ctx)> task);
	void SamplePhotonsFromScreen();
	void TracePhoton(ThreadContext& ctx, const PathSegment& emitted);
	static std::pair<int, int> GetDimensionsFromLongestEdge(const Camera& camera, int longestEdge);
	void SaveTransmission(ThreadContext& ctx, const PathSegment& current, PathSegment& outgoing);

	Pixel ComputeIndirectIllumination_MSAA(ThreadContext& ctx, const std::vector<PathSegment>& msaaView) const;
	WeightedPixel ComputeDirectIllumination_MSAA(ThreadContext& ctx, const std::vector<PathSegment>& msaaView) const;
	void ComputeDirectIllumination_Cluster(ThreadContext& ctx, const MSAACluster& cluster) const;
	void WritePerformanceData(const UVMapNPOT<int64_t>& data, std::string extension);
public:

	const RenderSettings& GetSettings() const { return settings; }

	void TraverseImage(Rect rect, std::function<void (int x, int y)> callback) const;

	RayTracer(std::shared_ptr<Scene> scene, RenderSettings settings);

	void TraverseScreenSpace(
		int xResolution,
		int yResolution,
		std::function<void (ThreadContext& ctx, ScreenSpacePosition& ssp)> perPixelCallback);

	const Camera& GetCamera() const { return camera; }

	void TracePhotons();

	void RenderImage();

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	boost::iterator_range<std::vector<Triangle>::const_iterator> GetTriangles() const { return boost::make_iterator_range(triangles.cbegin(), triangles.cend()); }

	int GetThreadCount() const { return (int)threadCtx.size(); }

	Pixel GetClearColor() const { return Pixel(0.5,0.5,0.5); }

	RenderBuffer& GetFrameBuffer() { return frameBuffer; }

	PhotonMap& GetIndirectMap() { return indirectMap; }
	PhotonMap& GetDirectMap() { return directMap; }
	PhotonMap& GetCausticsMap() { return causticsMap; }

	Pixel EstimateIndirectIllumination(ThreadContext& ctx, const PathSegment& view) const;
};

#endif