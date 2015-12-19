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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using UnityEditor;
using UnityEngine;

[ExecuteInEditMode]
public class RayliceMaterialCollector : MonoBehaviour
{
    public bool includeInactive;
    public GameObject dragAndDropSomething;
    public Material[] collectedMaterials;

    protected void Update()
    {
        if (dragAndDropSomething != null)
        {
            GameObject obj = dragAndDropSomething;
            dragAndDropSomething = null;

            Debug.Log("Scanning game object \"" + obj + "\" for materials...", obj);

            Scan(obj);
        }
    }

    public void Scan(GameObject obj)
    {
        var mats = new List<Material>();

        foreach (var renderer in obj.GetComponentsInChildren<MeshRenderer>(includeInactive))
        {
            foreach (var mat in renderer.sharedMaterials)
            {
                mats.Add(mat);
            }
        }

        foreach (var renderer in obj.GetComponentsInChildren<SkinnedMeshRenderer>(includeInactive))
        {
            foreach (var mat in renderer.sharedMaterials)
            {
                mats.Add(mat);
            }
        }

        collectedMaterials = mats.Distinct().ToArray();

        // TODO: remove this when all scenes have been converted...
        foreach (var mat in collectedMaterials)
        {
            var shader = mat.shader;

            if (shader.name != "RayTracingShader")
                continue;

            var diffuseMap = mat.GetTexture("LAMBERT_ColorMap");
            var diffuse = mat.GetColor("LAMBERT_Color");

            mat.shader = Shader.Find("Diffuse");
            mat.SetTexture("_MainTex", diffuseMap);
            mat.SetColor("_Color", diffuse);
        }
    }
}