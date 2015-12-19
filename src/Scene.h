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


class Mesh
{
	friend class RayTracer;
	friend struct Triangle;
	friend class UnityImporter;
private:
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	std::shared_ptr<BSDFMaterial> material;
	std::shared_ptr<LightSource> light;
public:

	LightSource* GetLight() const { return light.get(); }
	void SetLight(std::shared_ptr<LightSource> light) { this->light = light; }
	const Triangle& GetTriangleByIndex(int index) const { return triangles[index]; }
	BSDFMaterial* GetMaterial() const { return material.get(); }
	boost::iterator_range<std::vector<Vertex>::const_iterator> GetVertices() const { return boost::make_iterator_range(vertices.cbegin(), vertices.cend()); }
	boost::iterator_range<std::vector<Triangle>::const_iterator> GetTriangles() const { return boost::make_iterator_range(triangles.cbegin(), triangles.cend()); }
	boost::iterator_range<std::vector<Vertex>::iterator> GetVertices() { return boost::make_iterator_range(vertices.begin(), vertices.end()); }
	boost::iterator_range<std::vector<Triangle>::iterator> GetTriangles() { return boost::make_iterator_range(triangles.begin(), triangles.end()); }

	std::shared_ptr<Mesh> Instanciate(Matrix4x4 transform, std::shared_ptr<UnifiedSettings> material);
};

class LightSource
{
private:
	float photonMultiplier, intensity;
	std::string name;
	int index;
	std::shared_ptr<Mesh> mesh;
	std::map<double, const Triangle*> probToTriangle;
	double maximumProbability;
	Pixel color;
	std::shared_ptr<TextureMap> texture;
	bool isDirectional;
	std::shared_ptr<UnifiedSettings> settings;
public:
	LightSource(std::shared_ptr<Mesh> mesh);
	static std::shared_ptr<LightSource> TryFromTemplate(std::shared_ptr<UnifiedSettings> settings, std::shared_ptr<Mesh> mesh);

	int GetIndex() const { return index; }
	void SetIndex(int index) { this->index = index; }
	const std::string& GetName() const { return name; }

	float GetPhotonMultiplier() const { return photonMultiplier; }
	void SetPhotonMultiplier(float value) { photonMultiplier = value; }

	float GetIntensity() const { return intensity; }
	void SetIntensity(float value) { intensity = value; }

	bool IsDirectional() const { return isDirectional; }
	void SetDirectional(bool value) { isDirectional = value; }

	Pixel GetColor() const { return color; }
	void SetColor(Pixel value) { color = value; }

	const TextureMap& GetTexture() const { return *texture.get(); }
	void SetTexture(std::shared_ptr<TextureMap> value) { texture = value; }

	bool EmitPhoton(ThreadContext& ctx) const;
};

class Scene
{
protected:
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<Camera> cameras;
	std::vector<LightSource*> lights;

	Scene() { }
	virtual ~Scene() { }
public:

	boost::iterator_range<std::vector<Camera>::iterator> GetCameras() { return boost::make_iterator_range(cameras.begin(), cameras.end()); }
	boost::iterator_range<std::vector<std::shared_ptr<Mesh>>::iterator> GetMeshes() { return boost::make_iterator_range(meshes.begin(), meshes.end()); }
	boost::iterator_range<std::vector<LightSource*>::iterator> GetLights() { return boost::make_iterator_range(lights.begin(), lights.end()); }
};
