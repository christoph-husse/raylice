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


#ifndef _RAYTRACERFWD_H_
#define _RAYTRACERFWD_H_

class RayIntersector;
class Camera;
class BSDFMaterial;
class LambertMaterial;
struct SamplePoint;
class PhotonMap;
template<class TValue> class ProgressBar;
class LightSource;
class Mesh;
class Scene;
struct Pixel;
class TextureMap;
class UnityImporter;
class RayTracer;
class ThreadContext;
template<class TUVEntry> class UVMap;
class PathSegment;
class RenderSettings;
struct WeightedPixel;
struct MultiplicativeBSDFEntry;

#define _SCL_SECURE_NO_WARNINGS

#ifndef _WIN32
	#define __forceinline inline
	#define __declspec(unused)
#endif
	
#pragma warning (disable: 4100) // unreferenced parameter
#pragma warning (disable: 4505) // unreferenced local function
#pragma warning (disable: 4710) // function not inlined
#pragma warning (disable: 4514) // unreferenced inline function
#pragma warning (disable: 4820) // padding added
#pragma warning (disable: 4503) // decorated name truncadted

#pragma warning (push)

	#pragma warning (disable: 4706 4702 4512 4310 4267 4244 4917 4820 4710 4514 4365 4350 4640 4571 4127 4242 4350 4668 4626)
	#pragma warning (disable: 4625 4265 4371)

	#include <stdint.h>

	#include <iostream>
	#include <vector>
	#include <string>
	#include <map>
	#include <functional>
	#include <thread>
	#include <memory>
	#include <array>
	#include <set>
	#include <ctime>
	#include <random>
	#include <atomic>
	#include <mutex>
	#include <unordered_map>
	#include <fstream>
	#include <cstdlib>
	#include <iomanip>
	#include <cstring>
	#include <ctime>
	#include <cmath>

	#define BOOST_ALL_NO_LIB

	#include <boost/range/iterator_range.hpp>
	#include <boost/noncopyable.hpp>
	#include <boost/lexical_cast.hpp>
	#include <boost/optional.hpp>
	#include <boost/algorithm/string/split.hpp>
	#include <boost/algorithm/string/classification.hpp>

	#include "vectormathlibrary/include/vectormath/cpp/vectormath_aos.h"
#pragma warning (pop)


class StopWatch
{
	typedef std::chrono::high_resolution_clock clock;

private:
	clock::time_point start;
public:
	StopWatch() : start(clock::now()) { }

	void Reset() { start = clock::now(); }

	auto GetElapsed() const -> decltype(clock::time_point() - clock::time_point()) { return clock::now() - start; }
	std::chrono::milliseconds GetElapsedMillis() const { return std::chrono::duration_cast<std::chrono::milliseconds>(GetElapsed()); }
	std::chrono::seconds GetElapsedSeconds() const { return std::chrono::duration_cast<std::chrono::seconds>(GetElapsed()); }
	std::chrono::minutes GetElapsedMinutes() const { return std::chrono::duration_cast<std::chrono::minutes>(GetElapsed()); }
	std::chrono::hours GetElapsedHours() const { return std::chrono::duration_cast<std::chrono::hours>(GetElapsed()); }
};

inline std::ostream& operator <<(std::ostream& out, const StopWatch& watch)
{
	auto h = watch.GetElapsedHours().count();
	auto m = watch.GetElapsedMinutes().count();
	auto s = watch.GetElapsedSeconds().count();
	auto ms = watch.GetElapsedMillis().count();


	out <<h << ":" << (m - h * 60) << ":" << (s - (m + h * 60) * 60);
	return out;
}

template <unsigned int count, typename T> 
inline T rol(T val) 
{
	static_assert(std::is_unsigned<T>::value && std::is_integral<T>::value, "Only supported for unsigned integer types!");

	return (val << count) | (val >> (sizeof(T)*CHAR_BIT-count));
}

namespace std
{
	template<class T1, class T2>
	struct hash<pair<T1, T2>>
	{
		size_t operator()(const pair<T1, T2>& val) const
		{
			return hash<T1>()(val.first) ^ rol<sizeof(size_t) * 4>(hash<T1>()(val.second));
		}
	};

	template<typename T>
	std::unique_ptr<T> make_unique()
	{
		return std::move(std::unique_ptr<T>(new T()));
	}

	template<typename T, typename TArg1>
	std::unique_ptr<T> make_unique(TArg1&& arg1)
	{
		return std::move(std::unique_ptr<T>(new T(std::forward<TArg1>(arg1))));
	}

	template<typename T, typename TArg1, typename TArg2>
	std::unique_ptr<T> make_unique(TArg1&& arg1, TArg2&& arg2)
	{
		return std::move(std::unique_ptr<T>(new T(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2))));
	}

	template<typename T, typename TArg1, typename TArg2, typename TArg3>
	std::unique_ptr<T> make_unique(TArg1&& arg1, TArg2&& arg2, TArg3&& arg3)
	{
		return std::move(std::unique_ptr<T>(new T(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2), std::forward<TArg3>(arg3))));
	}

	template<class T>
	T sign(T value)
	{
		return (value > 0) ? 1 : ((value < 0) ? -1 : 0);
	}
}

#define MAYBE_UNUSED

#include "sys/ref.h"
#include "common/accel.h"
#include "common/intersector.h"

#include "nanoflann.hpp"

#undef assert

inline void assert(bool expr) 
{
#if defined(ENABLE_ASSERTIONS) || defined(_DEBUG)
	if(!expr)
	{
		std::cerr << std::endl << "ASSERTION FAILED!" << std::endl;
#ifdef _WIN32
		debugbreak();
#endif
		exit(-1);
	}
#endif
}

#endif