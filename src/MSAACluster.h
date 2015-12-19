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


struct MultiplicativeBSDFEntry
{
public:
	BSDF* bsdf;
	BSDFMaterial* material;
	PathSegment view;
	Pixel color;
	float probability;

	MultiplicativeBSDFEntry() { Reset(); } 

	void Reset()
	{
		probability = 1;
		bsdf = nullptr;
		material = nullptr;
		view = PathSegment();
		color = Pixel(1,1,1);
	}
};

class MSAACluster
{
private:
	bool isInitialized;
	float halfRadius;
public:
	std::vector<const PathSegment*> photons;
	std::vector<MultiplicativeBSDFEntry*> entries;

	MSAACluster() : isInitialized(false) { }

	bool IsInitialized() const { return isInitialized; }

	void Reset()
	{
		isInitialized = false;
		photons.clear();
		entries.clear();
		halfRadius = 0;
	}

	template<class TPhotons>
	void Initialize(MultiplicativeBSDFEntry* bsdfEntry, TPhotons photonSource) 
	{
		Reset();
		
		halfRadius = 0;
		for(const PathSegment* photon : photonSource)
		{
			/*
				Photons are gathered based on distance to first viewer, so usually
				we will get the center of the photon cloud. This means we are approx.
				computing the radius of the cloud here.
			*/
			halfRadius = std::max(halfRadius, Math::Length(photon->GetImpact() - bsdfEntry->view.GetImpact()));
			photons.push_back(photon);
		}

		halfRadius /= 2;
		isInitialized = true;
		entries.push_back(bsdfEntry);
	}

	bool Add(MultiplicativeBSDFEntry* bsdfEntry)
	{
		if(!entries.empty())
		{
			float distance = Math::Length(entries[0]->view.GetImpact() - bsdfEntry->view.GetImpact());

			if(distance > halfRadius)
				return false;
		}

		entries.push_back(bsdfEntry);
		return true;
	}

	MultiplicativeBSDFEntry* SelectEntry() const
	{
		return entries[std::rand() % entries.size()];
	}
};