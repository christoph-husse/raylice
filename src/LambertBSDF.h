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


class LambertBSDF : public LightingBSDFBase
{
private:
    float emissiveIntensity;
    bool isEmissive;
public:

	LambertBSDF();

	static LambertBSDF* TryFromTemplate(std::shared_ptr<BSDFMaterial> material);

	float GetEmissiveIntensity() const { return emissiveIntensity; }
	void SetEmissiveIntensity(float value) { emissiveIntensity = value; }

	bool IsEmissive() const { return isEmissive; }
	void SetEmissive(bool value) { isEmissive = value; }

	virtual PathSegment Transmit(ThreadContext& ctx, PathSegment& incoming) const override;
	virtual bool TransmitCamera(ThreadContext& ctx, const PathSegment& view, PathSegment* next) const override { return false; }
	virtual Pixel ComputeDirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const override;
	virtual Pixel ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const override;
};