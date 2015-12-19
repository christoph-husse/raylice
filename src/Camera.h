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


class Camera
{
public:
	std::string name;
	float aspect;
	Matrix4x4 transform;
	Matrix4x4 projection;
	Matrix4x4 worldToNDC;
public:

	Camera(std::string name, float aspect, Matrix4x4 transform, Matrix4x4 projection)
		: name(name), aspect(std::max(1.0f, aspect)), transform(transform), projection(projection)
	{
		worldToNDC = projection * transform;
	}

	const std::string& GetName() const { return name; }
	float GetAspect() const { return aspect; }
	Vector3 GetEyePosition() const { return Math::Convert(transform.getTranslation()); }

	/*
		This is all we need for ray-tracing. It gives us the camera eye coordinate, which will
		be our ray-origin, and it also gives us the two scaled plane axis. Scaled means that 
		they match the near plane's x and y sides in length. So we can just multiply those
		vector's each by normalized resolution dependent factors and then add them together to
		get the ray's raster point (where it passes through from the origin). This is pretty
		convenient!
	*/
	void GetRayRaster(
		Vector3& outNearOrigin, 
		Vector3& outNearXAxis, 
		Vector3& outNearYAxis,
		Vector3& outFarOrigin, 
		Vector3& outFarXAxis, 
		Vector3& outFarYAxis) const
	{
		Matrix4x4 ndcToWorld = Vectormath::Aos::inverse(projection * transform);

		outFarOrigin = Math::TransformVector(ndcToWorld, Vector3(-1,1,1));
		outFarXAxis = Math::TransformVector(ndcToWorld, Vector3(1,1,1));
		outFarYAxis = Math::TransformVector(ndcToWorld, Vector3(-1,-1,1));

		outNearOrigin = Math::TransformVector(ndcToWorld, Vector3(-1,1,-1));
		outNearXAxis = Math::TransformVector(ndcToWorld, Vector3(1,1,-1));
		outNearYAxis = Math::TransformVector(ndcToWorld, Vector3(-1,-1,-1));

		outFarXAxis = outFarXAxis - outFarOrigin;
		outFarYAxis = outFarYAxis - outFarOrigin;

		outNearXAxis = outNearXAxis - outNearOrigin;
		outNearYAxis = outNearYAxis - outNearOrigin;
	}

	Vector3 WorldToNDC(Vector3 world) const
	{
		return Math::TransformVector(worldToNDC, world);
	}
};
