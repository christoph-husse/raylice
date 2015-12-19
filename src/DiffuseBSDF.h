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



class DiffuseBSDF : public SurfaceBSDFBase
{
private:
	Pixel diffuse;
	std::shared_ptr<TextureMap> diffuseTexture;

public:

	DiffuseBSDF() : diffuse(Pixel(223, 20, 245) / 255.0), diffuseTexture() { }

	static DiffuseBSDF* TryFromTemplate(std::shared_ptr<BSDFMaterial> material)
	{
		auto mTemplate = material->GetTemplate();
		auto diffuse = mTemplate->TryGetInstancePath("Surface", "Diffuse");
		auto res = material->AddBSDF<DiffuseBSDF>();

		if(diffuse)
		{
			res->SetColor(diffuse->GetColor("Color"));
			res->SetDiffuseTexture(diffuse->GetTexture2D("Texture"));
		}

		return res;
	}

	Pixel GetColor() const { return diffuse; }
	void SetColor(const Pixel value) { diffuse = value; }

	std::shared_ptr<TextureMap> ShareDiffuseTexture() const { return diffuseTexture; }
	const TextureMap* GetDiffuseTexture() const { return diffuseTexture.get(); }
	void SetDiffuseTexture(std::shared_ptr<TextureMap> value) { diffuseTexture = value; }

	virtual Pixel ShadeSurface(const PathSegment& viewer) const override
	{
		Pixel diffuse = this->diffuse;

		if(diffuseTexture)
			diffuse *= (*diffuseTexture)(*viewer.GetTriangleAtImpact(), viewer.GetImpact());

		return diffuse;
	}

	virtual PathSegment Transmit(ThreadContext& ctx, PathSegment& incoming) const override
	{
		throw std::bad_exception("Surface BSDFs do not support photon transmission! Getting here may indicate a bug in the rendering code.");
	}

	virtual Pixel ComputeDirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const override
	{
		throw std::bad_exception("Surface BSDFs do not support direct lighting! Getting here may indicate a bug in the rendering code.");
	}

	virtual Pixel ComputeIndirectIllumination(ThreadContext& ctx, const PathSegment& viewer, const PathSegment& incomingLight) const override
	{
		throw std::bad_exception("Surface BSDFs do not support indirect lighting! Getting here may indicate a bug in the rendering code.");
	}
};