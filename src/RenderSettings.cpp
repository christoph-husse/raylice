#include "stdafx.h"

RenderSettings::RenderSettings()
{
	Initialize("draft");
}

RenderSettings RenderSettings::Empty()
{
	RenderSettings res;
	res.msaaSamples = -1;
	res.subSamples = -1;
	res.photonCount = -1;
	res.indirectSmoothingSamples = -1;
	res.indirectLightAmplifier = -1;
	res.indirectLightTolerance = -1;
	res.qualityPreset = "";
	res.photonIntensity = -1;
	res.emissiveIntensity = -1;
	res.resolution = -1;
	res.threadCount = -1;
	res.noPreview = false;
	res.shadowSampleFactor = -1;
	res.shadowSamples = -1;
	res.indirectLocalSamples = -1;

	return res;
}

void RenderSettings::ApplyDefaults(const RenderSettings& defaults)
{
	noPreview = defaults.noPreview;
	qualityPreset = defaults.qualityPreset;
	inputFile = defaults.inputFile;
	outputFile = defaults.outputFile;

	if(msaaSamples < 0) msaaSamples = defaults.msaaSamples;
	if(subSamples < 0) subSamples = defaults.subSamples;
	if(photonCount < 0) photonCount = defaults.photonCount;
	if(indirectLocalSamples < 0) indirectLocalSamples = defaults.indirectLocalSamples;
	if(indirectSmoothingSamples < 0) indirectSmoothingSamples = defaults.indirectSmoothingSamples;
	if(indirectLightAmplifier < 0) indirectLightAmplifier = defaults.indirectLightAmplifier;
	if(indirectLightTolerance < 0) indirectLightTolerance = defaults.indirectLightTolerance;
	if(photonIntensity < 0) photonIntensity = defaults.photonIntensity;
	if(emissiveIntensity < 0) emissiveIntensity = defaults.emissiveIntensity;
	if(resolution < 0) resolution = defaults.resolution;
	if(threadCount < 0) threadCount = defaults.threadCount;
	if(shadowSampleFactor < 0) shadowSampleFactor = defaults.shadowSampleFactor;
	if(shadowSamples < 0) shadowSamples = defaults.shadowSamples;
}

RenderSettings::RenderSettings(std::string qualityPreset)
{
	Initialize(qualityPreset);
}

void RenderSettings::Initialize(std::string qualityPreset)
{
	shadowSampleFactor = 0.25f;
	subSamples = 8;
	noPreview = false;
	threadCount = std::max(1, (int)std::thread::hardware_concurrency() - 1);
	indirectLightAmplifier = 2;
	indirectLightTolerance = 0.0001f;
	pixelPerfMonMask = EPixelPerfMon::None;
	pixelDebugMask = EPixelDebug::None;

	if(qualityPreset == "draft")
	{
		shadowSamples = 64;
		msaaSamples = 8;
		photonCount = 500000;
		photonIntensity = 1;
		emissiveIntensity = 1;
		resolution = 1024;
		indirectSmoothingSamples = 5;
		indirectLocalSamples = 32;
	}
	else if(qualityPreset == "normal")
	{
		shadowSamples = 128;
		msaaSamples = 16;
		photonCount = 1000000;
		photonIntensity = 1;
		emissiveIntensity = 1;
		resolution = 1024;
		indirectSmoothingSamples = 2;
		indirectLocalSamples = 64;
	}
	else if(qualityPreset == "high")
	{
		shadowSamples = 256;
		msaaSamples = 8;
		photonCount = 2000000;
		photonIntensity = 1;
		emissiveIntensity = 1;
		resolution = 1024;
		indirectSmoothingSamples = 4;
		indirectLocalSamples = 32;
	}
	else if(qualityPreset == "ultra")
	{
		shadowSamples = 512;
		msaaSamples = 48;
		photonCount = 4000000;
		photonIntensity = 1;
		emissiveIntensity = 1;
		resolution = 1920;
		indirectSmoothingSamples = 8;
		indirectLocalSamples = 256;
	}
	else 
	{
		std::cerr << "[WARNING]: Unrecognized quality preset. Switching to \"draft\"." << std::endl;
		Initialize("draft");
	}
}

bool RenderSettings::MakeValid()
{
	threadCount = std::max(1, std::min(threadCount, 64));
	msaaSamples = std::min(1024, std::max(msaaSamples, 1));
	indirectLocalSamples = std::min(1024, std::max(indirectLocalSamples, 1));
	shadowSamples = std::min(1024, std::max(shadowSamples, 1));
	indirectSmoothingSamples = std::min(64, std::max(indirectSmoothingSamples, 1));
	indirectLightAmplifier = std::min(10.0f, std::max(indirectLightAmplifier, 0.1f));
	indirectLightTolerance = std::max(0.00000001f, std::min(indirectLightTolerance, 1000.0f));
	subSamples = std::min(1024, std::max(subSamples, 1));
	photonCount = std::min(100000000, std::max(photonCount, 10000));
	resolution = std::min(4096, std::max(resolution, 64));
	photonIntensity = std::max(0.001f, std::min(photonIntensity, 1000.0f));
	emissiveIntensity = std::max(0.001f, std::min(emissiveIntensity, 1000.0f));

#ifdef _DEBUG
	shadowSampleFactor = 0.25f;
	shadowSamples = 32;
	msaaSamples = 2;
	subSamples = 8;
	photonCount = 10000;
	resolution = 256;
	threadCount = 3;
	indirectLocalSamples = 16;


#endif

	if(inputFile.empty())
	{
		std::cerr << "[ERROR]: Input file was not specified!" << std::endl;
		return false;
	}

	if(outputFile.empty())
		outputFile = inputFile + ".exr";

	return true;
}