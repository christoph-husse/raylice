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

#include <boost/program_options.hpp>


int processCommandLine(int argc, char** argv, RenderSettings& outSettings)
{
	namespace po = boost::program_options;

	std::cout << std::endl;

	// configure supported options
	po::options_description desc("The following parameters are supported:");
	desc.add_options()
		("help", "Show a list of supported parameters.")
		("msaa-samples,m", po::value<int>(), "Number of high-level MSAA samples (performance overhead compared to \"sub-samples\" options, but much higher quality (default value depends on quality level, ranging from 8 to 64)!")
		("sub-samples", po::value<int>(), "Number of internal indirect lighting samples per MSAA-sample (default value of 8 recommended, increase MSAA samples instead).")
		("thread-count,t", po::value<int>(), "Number of threads to run (default is N-1, where N is the number of physical cores).")
		("photon-count,p", po::value<int>(), "Number of photons to be emitted. Default value depends on quality level, ranging from 1 mio. to 10 mio.")
		("quality-preset,q", po::value<std::string>(), "Valid values are \"draft\", \"normal\", \"high\" and \"ultra\", default is \"draft\".")
		("photon-intensity", po::value<float>(), "Photon intensity factor by which the intensity predefined by a scene should be multiplied (default is one). Allows quick alternation of global brightness.")
		("emissive-intensity", po::value<float>(), "Emissive intensity factor by which the intensity predefined by a scene should be multiplied (default is one). Allows quick alternation of local brightness.")
		("shadow-samples", po::value<int>(), "How many photons should be traced back to a light source when computing the shadow for any given pixel?")
		("shadow-sample-factor", po::value<float>(), "Multiplied by \"shadow-samples\" to derive the shadow-samples for transmissive materials. (Default is 0.25)")
		("indirect-local-samples", po::value<int>(), "The amount of random estimates to collect for each indirect local sample. (defaults to 32-256 depedening on quality level)")
		("indirect-smoothing-samples", po::value<int>(), "Randomly select one nearest photon out of %ARG% many neighboring photons for indirect illumination estimation. (defaults to 1-8 depedening on quality level)")
		("indirect-light-amplifier", po::value<float>(), "Multiplicator for direct lighting used to estimate indirect lighting. Default is 2, higher values make shadow regions brighter.")
		("indirect-light-tolerance", po::value<float>(), "How close must a mutated light ray hit be to the current estimation point to be considered a light-hit? Default is 0.0001! Other values may be needed to accomodate strange model dimensions (precision issues).")
		("resolution,r", po::value<int>(), "Resolution in pixels of the final image (longest side, depending on aspect ratio of the scene's camera). Default is 1024.")
		("debug", po::value<std::string>(), "Outputs various debug files. Multiple values can be passed separated by a comma. Valid values are 'indirect', 'direct', 'estimate', 'local' and 'all'.")
		("perfmon", po::value<std::string>(), "Outputs various performance monitoring files. Multiple values can be passed separated by a comma. Valid values are 'indirect', 'direct', 'total' and 'all'.")
		("output-file,o", po::value<std::string>(), "The image file to be generated. Only OpenEXR file format is supported! Default value is input file followed by \".exr\".")
		("input-file,i", po::value<std::string>(), "A scene file to be rendered. This is the only required parameter!")
		("no-preview", "Don't show a preview window during rendering.")
	;

	// parse command line
	po::variables_map vm;
	
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);    
	}
	catch(...)
	{
		std::cout << desc << "\n";
		return 1;
	}

	if (vm.count("help")) 
	{
		std::cout << desc << "\n";
		return 1;
	}

	// gather settings
	outSettings.Initialize("draft");

	if (vm.count("quality-preset")) outSettings = RenderSettings(vm["quality-preset"].as<std::string>());
	if (vm.count("msaa-samples")) outSettings.msaaSamples = vm["msaa-samples"].as<int>();
	if (vm.count("sub-samples")) outSettings.subSamples = vm["sub-samples"].as<int>();
	if (vm.count("photon-count")) outSettings.photonCount = vm["photon-count"].as<int>();
	if (vm.count("photon-intensity")) outSettings.photonIntensity = vm["photon-intensity"].as<float>();
	if (vm.count("emissive-intensity")) outSettings.emissiveIntensity = vm["emissive-intensity"].as<float>();
	if (vm.count("shadow-samples")) outSettings.shadowSamples = vm["shadow-samples"].as<int>();
	if (vm.count("shadow-sample-factor")) outSettings.shadowSampleFactor = vm["shadow-sample-factor"].as<float>();
	if (vm.count("indirect-local-samples")) outSettings.indirectLocalSamples = vm["indirect-local-samples"].as<int>();
	if (vm.count("indirect-smoothing-samples")) outSettings.indirectSmoothingSamples = vm["indirect-smoothing-samples"].as<int>();
	if (vm.count("thread-count")) outSettings.threadCount = vm["thread-count"].as<int>();
	if (vm.count("indirect-light-amplifier")) outSettings.indirectLightAmplifier = vm["indirect-light-amplifier"].as<float>();
	if (vm.count("indirect-light-tolerance")) outSettings.indirectLightTolerance = vm["indirect-light-tolerance"].as<float>();
	if (vm.count("resolution")) outSettings.resolution = vm["resolution"].as<int>();
	if (vm.count("output-file")) outSettings.outputFile = vm["output-file"].as<std::string>();
	if (vm.count("input-file")) outSettings.inputFile = vm["input-file"].as<std::string>();
	
	if (vm.count("perfmon"))
	{
		std::vector<std::string> values;
		boost::split(values, vm["perfmon"].as<std::string>(), boost::algorithm::is_any_of(","));

		for(auto& e : values)
		{
			int flag = 0;

			if((e == "direct") || (e == "all")) flag |= EPixelPerfMon::Direct;
			if((e == "indirect") || (e == "all")) flag |= EPixelPerfMon::Indirect;
			if((e == "total") || (e == "all")) flag |= EPixelPerfMon::Total;
			
			if(flag == 0)
				std::cerr << "[WARNING]: Invalid perfmon flag \"" << e << "\" (ignored)." << std::endl;

			outSettings.pixelPerfMonMask |= flag;
		}
	}

	if (vm.count("debug"))
	{
		std::vector<std::string> values;
		boost::split(values, vm["debug"].as<std::string>(), boost::algorithm::is_any_of(","));

		for(auto& e : values)
		{
			int flag = 0;

			if((e == "direct") || (e == "all")) flag |= EPixelDebug::Direct;
			if((e == "indirect") || (e == "all")) flag |= EPixelDebug::Indirect;
			if((e == "estimate") || (e == "all")) flag |= EPixelDebug::Estimate;
			if((e == "local") || (e == "all")) flag |= EPixelDebug::Local;

			if(flag == 0) 
				std::cerr << "[WARNING]: Invalid debug flag \"" << e << "\" (ignored)." << std::endl;

			outSettings.pixelDebugMask |= flag;
		}
	}

	if (vm.count("input-file")) outSettings.inputFile = vm["input-file"].as<std::string>();

	outSettings.noPreview = vm.count("no-preview");

	// validate and correct mistakes if possible
	if(!outSettings.MakeValid())
	{
		std::cerr << "[FATAL-ERROR]: Chosen settings are invalid!\n";
		return -1;
	}

	// print settings
	std::cout << "Rendering settings are:" << std::endl;
	std::cout << "    > MSAA-Samples = " << outSettings.msaaSamples << std::endl;
	std::cout << "    > Sub-Samples = " << outSettings.subSamples << std::endl;
	std::cout << "    > Photon count = " << outSettings.photonCount << std::endl;
	std::cout << "    > Photon intensity = " << outSettings.photonIntensity << std::endl;
	std::cout << "    > Emissive intensity = " << outSettings.emissiveIntensity << std::endl;
	std::cout << "    > Indirect-Local-Samples = " << outSettings.indirectLocalSamples << std::endl;
	std::cout << "    > Indirect-Smoothing-Samples = " << outSettings.indirectSmoothingSamples << std::endl;
	std::cout << "    > Indirect-Light-Amplifier = " << outSettings.indirectLightAmplifier << std::endl;
	std::cout << "    > Indirect-Light_Tolerance = " << outSettings.indirectLightTolerance << std::endl;
	std::cout << "    > Shadow-Samples = " << outSettings.shadowSamples << std::endl;
	std::cout << "    > Shadow-Sample-Factor = " << outSettings.shadowSampleFactor << std::endl;
	std::cout << "    > Resolution = " << outSettings.resolution << std::endl;
	std::cout << "    > Thread count = " << outSettings.threadCount << std::endl;
	std::cout << "    > Input file = \"" << outSettings.inputFile << "\"" << std::endl;
	std::cout << "    > Output file = \"" << outSettings.outputFile << "\"" << std::endl << std::endl;
	
	return 0;
}

int main(int argc, char** argv)
{
	RunTest_UnifiedSettings();
//	SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);
	//return 0;
	int result;
	RenderSettings settings;
	if((result = processCommandLine(argc, argv, settings)) != 0)
		return result;

	StopWatch watch;
	std::shared_ptr<UnityImporter> scene;

	std::cout << "Loading scene...";

#ifndef _DEBUG
	try
	{
#endif
		scene = std::make_shared<UnityImporter>(settings.inputFile);
#ifndef _DEBUG
	}
	catch(const std::exception& e)
	{
		std::cerr << "[ERROR]: Could not load scene file. Make sure file exists!" << std::endl;
		std::cerr << " > Details: " << e.what() << std::endl;
		return -1;
	}
#endif

	std::cout << " [DONE, " << watch << "]" << std::endl;
	watch.Reset();
	std::cout << "Initializing raytracer...";

	RayTracer rayTracer(scene, settings);

	std::cout << " [DONE, " << watch << "]" << std::endl;
	watch.Reset();
	std::cout << "Tracing photons...";

	rayTracer.TracePhotons();

	std::cout << " [DONE, " << watch << "]" << std::endl;
	watch.Reset();

	std::shared_ptr<RenderPreviewWindow> preview;
	
	if(!settings.noPreview)
		preview = std::make_shared<RenderPreviewWindow>(rayTracer);

	std::cout << "Rendering image...";

	rayTracer.RenderImage();

	std::cout << " [DONE, " << watch << "]" << std::endl;
	std::cout << ">> Image has been saved to disk!" << std::endl;

    return EXIT_SUCCESS;
}