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


#ifndef _TEXTUREMAP_H_
#define _TEXTUREMAP_H_

#include "UVMap.h"

#include <string>

class TextureMap : public UVMapNPOT<Texel>
{
private:
	int channelCount;
public:
	TextureMap(int width, int height, int channelCount);

	int GetChannelCount() const { return channelCount; }
	void SetBytesRGBA(const std::vector<unsigned char>& bytes);

	float ExtractAlpha(const Texel& pixel) const;
	float ExtractHeight(const Texel& pixel) const;
	Pixel Interpolated(const Triangle& triangle, Vector3 w) const;
};

class RenderBuffer : public UVMapNPOT<Pixel>
{
public:
	RenderBuffer(int width, int height) : UVMapNPOT<Pixel>(width, height) { }

	void SaveToEXR(std::string fileName);
	void BlitToOpenGL(Rect rect) const;
};

#endif