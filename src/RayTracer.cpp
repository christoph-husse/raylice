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


void RayTracer::SamplePhotonsFromScreen()
{
	ProgressBar<int> progress(GetWidth() * GetHeight());
	const int MSAA_RESOLUTION = 16;
	std::chrono::high_resolution_clock timer;

	// allocate optional debug buffers
	std::unique_ptr<RenderBuffer> debugLocal, debugEstimate, debugIndirect, debugDirect;
	std::unique_ptr<UVMapNPOT<int64_t>> perfTotal;
	{
		const int w = GetWidth(), h = GetHeight();
		if(GetSettings().pixelDebugMask & EPixelDebug::Local) debugLocal = std::make_unique<RenderBuffer>(w, h);
		if(GetSettings().pixelDebugMask & EPixelDebug::Estimate) debugEstimate = std::make_unique<RenderBuffer>(w, h);
		if(GetSettings().pixelDebugMask & EPixelDebug::Indirect) debugIndirect = std::make_unique<RenderBuffer>(w, h);
		if(GetSettings().pixelDebugMask & EPixelDebug::Direct) debugDirect = std::make_unique<RenderBuffer>(w, h);

		if(GetSettings().pixelPerfMonMask & EPixelPerfMon::Total) perfTotal = std::make_unique<UVMapNPOT<int64_t>>(w, h);
	}

	// actual rendering
	TraverseScreenSpace(
		GetWidth(),
		GetHeight(),
		[&](ThreadContext& ctx, ScreenSpacePosition ssp)
		{
			WeightedPixel direct, indirect, mean;	
			auto perfMark_Start = timer.now();
			const int x = ssp.xScreen, y = ssp.yScreen;
			int passes = 0;
			Pixel lastMean;
			float delta;
			Pixel directPixel, indirectPixel;

			do
			{
				passes++;
				lastMean = (Pixel)mean;

				ctx.msaaSamples.clear();

				for(int i = 0; i < settings.msaaSamples; i++)
				{
					// collect MSAA coordinates
					float xMsaa = ssp.xDelta / MSAA_RESOLUTION;
					float yMsaa = ssp.yDelta / MSAA_RESOLUTION;
					PathSegment screenSegment;
					Ray ray(ssp.GetRay(ssp.xNdc + xMsaa * (std::rand() % MSAA_RESOLUTION), ssp.yNdc + yMsaa * (std::rand() % MSAA_RESOLUTION)));

					screenSegment.SetDirection(ray.dir);
					screenSegment.SetOrigin(ray.org);
					screenSegment.SetTriangleAtImpact(nullptr);

					if(!ctx.CastRay(screenSegment, screenSegment))
					{
						direct += WeightedPixel(1, GetClearColor());
						indirect += WeightedPixel(1, GetClearColor());
					}
					else
					{
						ctx.msaaSamples.emplace_back(screenSegment);
					}
				}

				// render the screen-pixel
				mean += WeightedPixel(1, (Pixel)(ComputeDirectIllumination_MSAA(ctx, ctx.msaaSamples) + direct));
				delta = std::abs(Math::MaxElem((Pixel)mean - lastMean));

			#ifdef _DEBUG
				break;
			#endif

			}while(((passes < 2) || (delta > 0.001 * Math::MaxElem((Pixel)mean))) && (passes < 100));

			frameBuffer(x, y) = directPixel = (Pixel)mean;

			passes = 0;
			mean = WeightedPixel();

			do
			{
				passes++;
				lastMean = (Pixel)mean;

				ctx.msaaSamples.clear();

				for(int i = 0; i < settings.msaaSamples; i++)
				{
					// collect MSAA coordinates
					float xMsaa = ssp.xDelta / MSAA_RESOLUTION;
					float yMsaa = ssp.yDelta / MSAA_RESOLUTION;
					PathSegment screenSegment;
					Ray ray(ssp.GetRay(ssp.xNdc + xMsaa * (std::rand() % MSAA_RESOLUTION), ssp.yNdc + yMsaa * (std::rand() % MSAA_RESOLUTION)));

					screenSegment.SetDirection(ray.dir);
					screenSegment.SetOrigin(ray.org);
					screenSegment.SetTriangleAtImpact(nullptr);

					if(!ctx.CastRay(screenSegment, screenSegment))
					{
						direct += WeightedPixel(1, GetClearColor());
						indirect += WeightedPixel(1, GetClearColor());
					}
					else
					{
						ctx.msaaSamples.emplace_back(screenSegment);
					}
				}

				mean += WeightedPixel(1, (Pixel)(WeightedPixel(ctx.msaaSamples.size(), ctx.msaaSamples.size() * ComputeIndirectIllumination_MSAA(ctx, ctx.msaaSamples)) + indirect));
				delta = std::abs(Math::MaxElem((Pixel)mean - lastMean));

				if(passes >= 2)
				{
					float indirectContribution = Math::MaxElem((Pixel)mean / std::max(0.0001f, Math::MaxElem(directPixel)));

					if(indirectContribution < 0.1)
						break; // not worth refining this...
				}

			#ifdef _DEBUG
				break;
			#endif

			}while(((passes < 2) || (delta > 0.001 * Math::MaxElem((Pixel)mean))) && (passes < 100));

			frameBuffer(x, y) += indirectPixel = (Pixel)mean;

			auto perfMark_End = timer.now();


			// store performance data
			if(perfTotal) (*perfTotal)(x, y) = std::chrono::duration_cast<std::chrono::nanoseconds>(perfMark_End - perfMark_Start).count();

			// store debug data
			if(debugLocal) (*debugLocal)(x, y) = ctx.msaaSamples.front().GetMaterialAtImpact()->ComputeLocalIllumination(ctx, ctx.msaaSamples.front());
			if(debugEstimate) (*debugEstimate)(x, y) = (Pixel)EstimateIndirectIllumination(ctx, ctx.msaaSamples.front());
			if(debugIndirect) (*debugIndirect)(x, y) = indirectPixel;
			if(debugDirect) (*debugDirect)(x, y) = directPixel;

			progress += 1;
		});

	if(perfTotal) WritePerformanceData(*perfTotal.get(), ".perf_total.exr"); 

	if(debugLocal) debugLocal->SaveToEXR(GetSettings().outputFile + ".debug_local.exr");
	if(debugEstimate) debugEstimate->SaveToEXR(GetSettings().outputFile + ".debug_estimate.exr");
	if(debugIndirect) debugIndirect->SaveToEXR(GetSettings().outputFile + ".debug_indirect.exr");
	if(debugDirect) debugDirect->SaveToEXR(GetSettings().outputFile + ".debug_direct.exr");
}

void RayTracer::WritePerformanceData(const UVMapNPOT<int64_t>& data, std::string extension)
{
	const int w = data.GetWidth(), h = data.GetHeight();
	RenderBuffer rgb(w, h);

	// find min/max
	int64_t min = data(0,0), max = min;
	TraverseImage(Rect(0, 0, w, h), [&](int x, int y)
	{
		auto d = data(x, y);
		min = std::min(min, d);
		max = std::max(max, d);
	});

	// normalize to RGB
	TraverseImage(Rect(0, 0, w, h), [&](int x, int y)
	{
		auto d = data(x, y);
		auto norm = (d - min) / (float)(max - min);
		rgb(x, y) = Pixel(norm, 1 - norm, 0, d);
	});

	rgb.SaveToEXR(GetSettings().outputFile + extension);
}

void RayTracer::TracePhoton(ThreadContext& ctx, const PathSegment& emitted)
{
	ctx.transmissionsSwap.clear();
	ctx.transmissionsSwap.push_back(std::make_pair(emitted, emitted));

	while(!ctx.transmissionsSwap.empty())
	{
		ctx.transmissions.clear();

		for(const auto currentTrans : ctx.transmissionsSwap)
		{
			auto current = currentTrans.second;

			if(current.IsAlive())
			{
				current.GetMaterialAtImpact()->Transmit(ctx, current, ctx.transmissions);
			}
		}

		for(auto& trans : ctx.transmissions)
		{
			SaveTransmission(ctx, trans.first, trans.second);
		}

		std::swap(ctx.transmissions, ctx.transmissionsSwap);
	}
}

void RayTracer::SaveTransmission(ThreadContext& ctx, const PathSegment& current, PathSegment& outgoing)
{
	if(outgoing.IsAlive() && ctx.CastRay(current, outgoing))
	{
		PathSegment* allocated;
		
		if(outgoing.GetLight())
		{
			allocated = GetDirectMap().Insert(outgoing);

			if(allocated)
				GetIndirectMap().Register(allocated);
		}
		else
			allocated = GetIndirectMap().Insert(outgoing);

		if(allocated)
		{
			assert(current.GetAddress());

			allocated->SetPrevSegment(current.GetAddress());
			outgoing = *allocated;
		}
	}
	else
		outgoing.Kill();
}
