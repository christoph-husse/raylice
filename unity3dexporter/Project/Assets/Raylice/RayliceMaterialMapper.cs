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

using Raylice;
using UnityEngine;

[ExecuteInEditMode]
public class RayliceMaterialMapper : MonoBehaviour
{
    public RayliceMaterial rayliceMaterial;
    public Material[] unityMaterialsToMap;

    internal UnifiedSettings GetMappedMaterial(Material unityMaterial)
    {
        foreach(var unityMat in unityMaterialsToMap)
        {
            if (unityMat == unityMaterial)
                return rayliceMaterial.Settings;
        }

        return null;
    }
}