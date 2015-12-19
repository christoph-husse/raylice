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

#include <string>

namespace EPixelPerfMon
{
	enum
	{
		None = 0,
		Direct = 1,
		Indirect = 2,
		Total = 4
	};
};

namespace EPixelDebug
{
	enum
	{
		None = 0,
		Direct = 1,
		Indirect = 2,
		Local = 4,
		Estimate = 8
	};
};

class RenderSettings
{
public:
	int msaaSamples;
	int subSamples;
	int photonCount;
	int indirectSmoothingSamples;
	int indirectLocalSamples;
	float indirectLightAmplifier;
	float indirectLightTolerance;
	std::string qualityPreset;
	float photonIntensity;
	float emissiveIntensity;
	int resolution;
	int threadCount;
	std::string outputFile;
	std::string inputFile;
	bool noPreview;
	float shadowSampleFactor;
	int shadowSamples;
	int pixelPerfMonMask;
	int pixelDebugMask;

	RenderSettings();

	static RenderSettings Empty();

	void ApplyDefaults(const RenderSettings& defaults);

	RenderSettings(std::string qualityPreset);

	void Initialize(std::string qualityPreset);

	bool MakeValid();
};
