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

float PhotonMap::kdtree_distance(const float* p1, const size_t idx_p2, size_t size) const
{
	auto d0= p1[0] - registeredPhotons[idx_p2]->GetImpact().x;
	auto d1= p1[1] - registeredPhotons[idx_p2]->GetImpact().y;
	auto d2= p1[2] - registeredPhotons[idx_p2]->GetImpact().z;
	return d0*d0+d1*d1+d2*d2;
}

float PhotonMap::kdtree_get_pt(const size_t idx, int dim) const
{
	if (dim == 0) return registeredPhotons[idx]->GetImpact().x;
	else if (dim == 1) return registeredPhotons[idx]->GetImpact().y;
	else return registeredPhotons[idx]->GetImpact().z;
}

void PhotonMap::Register(PathSegment* photon) 
{ 
	assert(photon);

	int iRegister = photonRegisterIndex++;

	if(iRegister < registeredPhotons.size())
	{
		while(registeredPhotons.size() <= iRegister) registeredPhotons.push_back(nullptr);
		registeredPhotons[iRegister] = photon;
	}
	else
		photonRegisterIndex--;
}

PathSegment* PhotonMap::Insert(PathSegment& segment)
{
	int index = photonStorageIndex++;
	int iRegister = photonRegisterIndex++;
	if((photonStorage.size() > index) && (registeredPhotons.size() > iRegister))
	{
		PathSegment* photon = &photonStorage[index];
		segment.address = photon;
		*photon = segment;

		while(registeredPhotons.size() <= iRegister) registeredPhotons.push_back(nullptr);
		registeredPhotons[iRegister] = photon;
		return photon;
	}
	else
	{
		photonStorageIndex--;
		photonRegisterIndex--;
		return nullptr;
	}
}

PhotonMap::PhotonMap(RayTracer& rayTracer, int totalPhotonCount) 
	: 
		rayTracer(rayTracer), 
		totalPhotonCount(totalPhotonCount),
		photonStorage(totalPhotonCount),
		registeredPhotons(totalPhotonCount),
		photonStorageIndex(0),
		photonRegisterIndex(0)
{
}


void PhotonMap::Build()
{
	kdTree = std::make_shared<KDTree>(3, *this, nanoflann::KDTreeSingleIndexAdaptorParams(10));

	if(GetRegisteredPhotonCount() > 0)
	{
		kdTree->buildIndex();
	}
}

void PhotonMap::Sample(Vector3 where, int sampleCount, PhotonMapSearch& result)
{
	const float _where[3] = {where.x, where.y, where.z};

	result.Initialize(sampleCount);

	kdTree->knnSearch(_where, sampleCount, result.indices.data(), result.distances.data());

	for(int i = 0; i < result.indices.size(); i++)
	{
		result.photons[i] = registeredPhotons[result.indices[i]];
	}
}
