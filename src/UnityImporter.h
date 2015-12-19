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


class UnityImporter : public Scene
{
private:

	std::ifstream stream;
	BinaryReader reader;

	std::shared_ptr<UnifiedSettings> settings;
	std::vector<float> triangleBuffer;
	std::unordered_map<int64_t, std::shared_ptr<TextureMap>> texIdToTex;
	std::vector<unsigned char> textureBuffer;
	std::unordered_map<int64_t, std::shared_ptr<Mesh>> meshIdToMesh;

	void DeserializeMaterial(std::shared_ptr<Mesh> mesh);
	std::shared_ptr<Mesh> DeserializeTriangles(int64_t fileOffset);
	void DeserializeScene();
	std::shared_ptr<TextureMap> ReadTexture(int64_t fileOffset);
	void ReadSync();
public:
	UnityImporter(std::string file);
};
