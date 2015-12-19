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
using Raylice;
using UnityEditor;
using UnityEngine;
using System.Linq;
using System.Collections;

[CustomEditor(typeof(RayliceMaterialCollector))]
public class RayliceMaterialCollectorEditor : Editor
{
    public override void OnInspectorGUI()
    {
        var collector = target as RayliceMaterialCollector;

        if(GUILayout.Button("Collect Unmapped Materials In Scene"))
        {
            var unmapped = new List<Material>();
            var ctrl = RayliceController.Get(collector.gameObject);

            foreach(GameObject node in FindObjectsOfType(typeof(GameObject)))
            {
                var renderer = node.GetComponent<Renderer>();
                var skinnedRenderer = node.GetComponent<SkinnedMeshRenderer>();

                if (renderer != null)
                    TestMapping(ctrl, renderer.sharedMaterials, unmapped);

                if (skinnedRenderer != null)
                    TestMapping(ctrl, skinnedRenderer.sharedMaterials, unmapped);
            }

            collector.collectedMaterials = unmapped.ToArray();
        }

        DrawDefaultInspector();

        if (GUI.changed)
            EditorUtility.SetDirty(target);
    }


    private void TestMapping(RayliceController ctrl, IEnumerable<Material> mats, List<Material> outUnmapped)
    {
        foreach(var mat in mats)
        {
            if(ctrl.GetMappedMaterial(mat) == null)
                outUnmapped.Add(mat);
        }
    }
}