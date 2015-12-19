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

/*
	There are various results possible when it comes to transmissive
	ray casting. Here all possible results are listed.
*/
enum class ETransmissionResult
{
	/** Transmission successfully hit a non-transmissive BSDF */
	Success,
	/** Transmission leads to empty space */
	EmptySpace,
	/** Transmission was not along a straight line */
	Dispersed,
	/** Transmission was aborted, because bouncing limit was reached. */
	NotAlive,
};


class ThreadContext
{
private:
	friend class RayTracer;
	friend class PhotonMap;

	const int threadIndex;
	RayTracer* const rayTracer;
	std::vector<std::shared_ptr<std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>>> freeBsdfGroups;
	std::vector<std::shared_ptr<MultiplicativeBSDFEntry>> freeBsdfEntries;

	void SetIntersector(std::shared_ptr<RayIntersector> intersector);
	void PrepareRunParallel();

	ThreadContext& operator=(const ThreadContext& rhs);
public:

	std::vector<int> forbiddenTriangles;
	PhotonMapSearch samples;
	std::shared_ptr<RayIntersector> intersector;
	std::vector<std::pair<PathSegment, PathSegment>> transmissions, transmissionsSwap;
	std::vector<PathSegment> msaaSamples;
	std::vector<BSDFMaterial*> msaaMaterials;
	std::vector<MSAACluster> msaaClusters;
	std::vector<WeightedPixel> pixels;
	std::vector<std::shared_ptr<std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>>> bsdfGroups;

	ThreadContext(RayTracer* tracer, int threadIndex);

	std::shared_ptr<MultiplicativeBSDFEntry> AllocateBsdfEntry(MultiplicativeBSDFEntry init = MultiplicativeBSDFEntry());
	std::shared_ptr<std::vector<std::shared_ptr<MultiplicativeBSDFEntry>>> AllocateBsdfGroup();
	void ResetBSDFGroups();

	bool CastRay(const PathSegment& incoming, PathSegment& outgoing);
	void ResetMSAAClusters();
	const RenderSettings& GetSettings() const;
	int GetThreadIndex() const { return threadIndex; }
	RayTracer* GetTracer() const { return rayTracer; }
	Pixel GetClearColor() const;
	PhotonMap& GetIndirectMap();
	PhotonMap& GetDirectMap();
	PhotonMap& GetCausticsMap();

	/*
		Follows the ray designated by "line" on a straight line through transmissive
		materials. If we hit a refractive/reflective material it is likely that the
		transmission does not continue this line, in which case this method would
		abort and return false! 
	*/
	bool FollowTransmissive(PathSegment& view, WeightedPixel* addToColor, BSDF** outBsdf = nullptr)
	{
		Pixel color;
		bool res = FollowTransmissive(view, &color, outBsdf); 
		if(!res)
			*addToColor += WeightedPixel(1, color);
		return res;
	}
	bool FollowTransmissive(PathSegment& view, Pixel* addToColor, BSDF** outBsdf = nullptr);
	ETransmissionResult FollowTransmissiveEx(PathSegment& view, BSDF** outBsdf = nullptr, bool onStraightLine = false);
	ETransmissionResult FollowTransmissiveLine(PathSegment& line, BSDF** outBsdf = nullptr) { return FollowTransmissiveEx(line, outBsdf, true); }
};
