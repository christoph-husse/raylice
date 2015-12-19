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


/**
	Storage class for search results of a PhotonMap. There exists only one per thread.
*/
struct PhotonMapSearch
{
private:
	friend class PhotonMap;

	std::vector<size_t> indices;
	std::vector<float> distances;
	std::vector<PathSegment*> photons;

	void Initialize(int maxSamples) 
	{
		indices.clear();
		distances.clear();
		photons.clear();

		indices.resize(maxSamples);
		distances.resize(maxSamples);
		photons.resize(maxSamples);
	}
public:
	PhotonMapSearch() : indices(), distances(), photons() 
	{ 
	}

	std::vector<PathSegment*>::const_iterator begin() { return photons.cbegin(); }
	std::vector<PathSegment*>::const_iterator end() { return photons.cend(); }
	PathSegment* Select() const { return photons[std::rand() % photons.size()]; }
};

/**
	A photon map provides the backing memory as well as a KD-tree based nearest neighbor
	search for PathSegments ("photons"). 
*/
class PhotonMap : boost::noncopyable
{
private:
	typedef nanoflann::L2_Simple_Adaptor<float, PhotonMap> Metric;
	typedef nanoflann::KDTreeSingleIndexAdaptor<Metric, PhotonMap, 3> KDTree;
	friend KDTree;
	friend Metric;

	std::shared_ptr<KDTree> kdTree;
	std::vector<PathSegment> photonStorage;
	RayTracer& rayTracer;
	const int totalPhotonCount;
	std::atomic<int> photonStorageIndex;
	std::atomic<int> photonRegisterIndex;
	std::vector<PathSegment*> registeredPhotons;

	inline size_t kdtree_get_point_count() const { return photonRegisterIndex; }

	float kdtree_distance(const float* p1, const size_t idx_p2, size_t size) const;

	float kdtree_get_pt(const size_t idx, int dim) const;

	template <class BBOX> bool kdtree_get_bbox(BBOX &bb) const { return false; }

public:
	PhotonMap(RayTracer& rayTracer, int totalPhotonCount);

	int GetTotalPhotonCount() const { return totalPhotonCount; }
	int GetStoredPhotonCount() const { return photonStorageIndex; }
	int GetRegisteredPhotonCount() const { return photonRegisterIndex; }
	PathSegment* Insert(PathSegment& segment);

	/**
		Only registers a photon for KD-tree search, but does not allocate memory for it.
		This is used when a photon is to be entered into different PhotonMaps, because
		each photon should only exist once in memory.
	*/
	void Register(PathSegment* photon);

	/**
		Can photons still be allocated? 
	*/
	bool HasFreeSpace() const { return photonStorageIndex < totalPhotonCount; }

	/**
		Build the KD-tree from all allocated & registered photons we have so far.
		All photons allocated/registered afterwards are not entered into the KD-tree!
	*/
	void Build();

	/**
		Only works after Build() has been called. Will sample exactly "sampleCount" many
		photons in the proximity of "where" (unless there are fewer photons in the map
		than requested!). 
	*/
	void Sample(Vector3 where, int sampleCount, PhotonMapSearch& result);

	std::vector<PathSegment>::const_iterator begin() const { return photonStorage.cbegin(); }
	std::vector<PathSegment>::const_iterator end() const { return photonStorage.cend(); }
};
