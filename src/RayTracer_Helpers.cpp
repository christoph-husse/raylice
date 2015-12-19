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

Pixel::Pixel(const WeightedPixel& src)
{
	*this = src.color / std::max(src.weight, 0.0001f);
}

std::pair<int, int> RayTracer::GetDimensionsFromLongestEdge(const Camera& camera, int longestEdge)
{
	int width, height;

	if(camera.GetAspect() > 1)
	{
		width = longestEdge;
		height = (int)(longestEdge / camera.GetAspect());
	}
	else
	{
		width = (int)(longestEdge * camera.GetAspect());
		height = longestEdge;
	}

	return std::make_pair(width, height);
}

RayTracer::RayTracer(std::shared_ptr<Scene> scene, RenderSettings settings)
	:
	settings(settings),
	camera(scene->GetCameras().front()),
	scene(scene),
	width(GetDimensionsFromLongestEdge(scene->GetCameras().front(), settings.resolution).first),
	height(GetDimensionsFromLongestEdge(scene->GetCameras().front(), settings.resolution).second),
	indirectMap(*this, settings.photonCount),
	causticsMap(*this, indirectMap.GetTotalPhotonCount()),
	frameBuffer(width, height),
	directMap(*this, indirectMap.GetTotalPhotonCount())
{
	if(std::distance(scene->GetLights().begin(), scene->GetLights().end()) == 0)
		std::invalid_argument("A scene needs at least one light source!");

	// create global map of all triangles and their materials
	for(auto& mesh : scene->GetMeshes())
	{
		BSDFMaterial* mat = mesh->GetMaterial();

		mat->ApplyDefaultSettings(settings);

		for(auto& tri : mesh->GetTriangles())
		{
			tri.index = triangles.size();
			triangles.push_back(tri);
			triToMatMap.push_back(mat);
		}
	}

	// create thread-contexts for further computational stages
	for(int iThread = 0; iThread < settings.threadCount; iThread++)
	{
		threadCtx.emplace_back(ThreadContext(this, iThread));
	}

	// creates spatial data structure for the triangles above, with full parallelization.
	auto intersector = std::make_shared<RayIntersector>(this);
	for(auto& ctx : threadCtx)ctx.SetIntersector(intersector);
}


void RayTracer::RunParallel(std::function<void (ThreadContext& ctx)> task)
{
	auto threads = std::vector<std::thread>();

	if(GetThreadCount() == 1)
	{
		// better stack traces if not artificially "parallelized"...
		task(threadCtx[0]);
	}
	else
	{
		for(auto& ctx : threadCtx)
		{
			threads.emplace_back(std::thread([&]()
			{
				task(ctx);
			}));
		}

		for(auto& t : threads) t.join();
	}
}

void RayTracer::TraverseImage(Rect rect, std::function<void (int x, int y)> callback) const
{
	for(int x = rect.left; x < rect.left + rect.width; x++)
	{
		for(int y = rect.top; y < rect.top + rect.height; y++)
		{
			callback(x, y);
		} 
	}
}

void RayTracer::TraverseScreenSpace(
	int xResolution,
	int yResolution,
	std::function<void (ThreadContext& ctx, ScreenSpacePosition& ssp)> perPixelCallback)
{
	// setup screen ray generator
	Vector3 nearOrigin, nearXAxis, nearYAxis, farOrigin, farXAxis, farYAxis;
	camera.GetRayRaster(nearOrigin, nearXAxis, nearYAxis, farOrigin, farXAxis, farYAxis);
	std::vector<Rect> blocks;

	auto GetScreenRay = [&](float xn, float yn) -> Ray
	{
		auto near = Vector3(nearOrigin + nearXAxis * xn + nearYAxis * yn);
		auto far = Vector3(farOrigin + farXAxis * xn + farYAxis * yn);
		return Ray(near, Math::Normalized(far - near));
	};

	// subdivide image screen into small processing blocks
	int minBlocks = 4 * (int)std::ceil(std::sqrtf(GetThreadCount()));
	int xStep = std::max(minBlocks, xResolution / 32);
	int yStep = std::max(minBlocks, yResolution / 32);
	for(int x = 0; x < xResolution; x += xStep)
	{
		int bWidth = std::min(xStep, xResolution - x); 

		for(int y = 0; y < yResolution; y += yStep)
		{
			int bHeight = std::min(yStep, yResolution - y);
			blocks.push_back(Rect(x, y, bWidth, bHeight));
		} 
	}

	// process all processing blocks
	std::atomic<int> blockIndex(0);

	RunParallel([&](ThreadContext& ctx)
	{
		int localIndex;
		while((localIndex = blockIndex++) < blocks.size())
		{
			Rect block = blocks[localIndex];

			TraverseImage(block, [&](int x, int y)
			{
				ScreenSpacePosition ssp;

				ssp.xDelta = (1 / (float)xResolution);
				ssp.yDelta = (1 / (float)yResolution);
				ssp.xNdc = x * ssp.xDelta;
				ssp.yNdc = y * ssp.yDelta;
				ssp.xScreen = x;
				ssp.yScreen = y;
				ssp.ray = GetScreenRay(ssp.xNdc, ssp.yNdc);
				ssp.GetRay = GetScreenRay;

				perPixelCallback(ctx, ssp);
			});
		}
	});
}

void RayTracer::TracePhotons()
{
	std::vector<int> photonCounts;

	for(const auto& light : scene->GetLights())
	{
		photonCounts.push_back(100);
	}

	ProgressBar<int> progress(indirectMap.GetTotalPhotonCount());

	RunParallel([&](ThreadContext& ctx)
	{
		while(indirectMap.GetRegisteredPhotonCount() < indirectMap.GetTotalPhotonCount())
		{
			for(const auto light : scene->GetLights())
			{
				int iLight = light->GetIndex();

				for(int i = 0; i < photonCounts[iLight]; i++)
				{
					light->EmitPhoton(ctx);
				}

				progress = indirectMap.GetRegisteredPhotonCount();
			}
		}
	});
}

void RayTracer::RenderImage()
{
	indirectMap.Build();
	directMap.Build();
	causticsMap.Build();

	SamplePhotonsFromScreen();

	frameBuffer.SaveToEXR(settings.outputFile);
}