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



class FresnelReflectance : public BSDFVisibility
{
private:
	float refractiveIndex;

public:

	FresnelReflectance() : refractiveIndex(1.5) {}

	void SetRefractiveIndex(float value) { refractiveIndex = Math::Clamp(value, 1.1f, 5.0f); }
	float GetRefractiveIndex() const { return refractiveIndex; }

	virtual float Get(const PathSegment& incoming) const override;
};

class FresnelRefractance : public BSDFVisibility
{
private:
	float refractiveIndex;

public:
	FresnelRefractance() : refractiveIndex(1.5) {}

	void SetRefractiveIndex(float value) { refractiveIndex = Math::Clamp(value, 1.1f, 5.0f); }
	float GetRefractiveIndex() const { return refractiveIndex; }

	virtual float Get(const PathSegment& incoming) const override;
};

class TransmissiveBSDF : public CausticBSDFBase
{
private:
	float alpha;
	std::shared_ptr<TextureMap> alphaMap;
public:

	TransmissiveBSDF() { }

	static TransmissiveBSDF* TryFromTemplate(std::shared_ptr<BSDFMaterial> material);

	virtual PathSegment Transmit(ThreadContext& ctx, PathSegment& incoming) const override;

	virtual Pixel ComputeDirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const override
	{
		throw std::bad_exception("Transmissive BSDFs do not support direct lighting! Getting here may indicate a bug in the rendering code.");
	}

	virtual Pixel ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const override
	{
		throw std::bad_exception("Transmissive BSDFs do not support invariant material colors! Getting here may indicate a bug in the rendering code.");
	}
};

class ReflectiveOrRefractiveBSDF : public TransmissiveBSDF
{
private:
	float refractiveIndex;
	float glossyPhi;

protected:
	void FromTemplate(std::shared_ptr<BSDFMaterial> material, std::shared_ptr<UnifiedSettings> caustic);

public:

	ReflectiveOrRefractiveBSDF() : refractiveIndex(2.5), glossyPhi(0) { SetGlossyAngleDegrees(5); }

	void SetRefractiveIndex(float value) { refractiveIndex = Math::Clamp(value, 1.1f, 5.0f); }
	float GetRefractiveIndex() const { return refractiveIndex; }

	void SetGlossyAngleDegrees(float value) { glossyPhi = Math::Clamp(value, 0.f, 90.f) * Math::PI / 180.f; }
	void SetGlossyAngleRadians(float value) { glossyPhi = Math::Clamp(value, 0.f, Math::PI / 2); }
	float GetGlossyAngle() const { return glossyPhi; }
};

class ReflectiveBSDF : public ReflectiveOrRefractiveBSDF
{
public:

	static ReflectiveBSDF* TryFromTemplate(std::shared_ptr<BSDFMaterial> material);

	virtual PathSegment Transmit(ThreadContext& ctx, PathSegment& incoming) const override;
};

class RefractiveBSDF : public ReflectiveOrRefractiveBSDF
{
public:

	static RefractiveBSDF* TryFromTemplate(std::shared_ptr<BSDFMaterial> material);

	virtual PathSegment Transmit(ThreadContext& ctx, PathSegment& incoming) const override;
};
