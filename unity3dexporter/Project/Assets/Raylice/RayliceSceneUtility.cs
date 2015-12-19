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
using System.IO;
using System.Security.AccessControl;
using System.Text;
using System.Threading;
using Raylice;
using UnityEngine;
using System.Linq;
using System.Collections;
using Object = UnityEngine.Object;

public class RayliceSceneUtility
{
    public static List<RayliceMesh> CollectGeometry(RayliceController ctrl)
    {
        var resut = new List<RayliceMesh>();

        foreach (GameObject current in Object.FindObjectsOfType(typeof(GameObject)))
        {
            // serialize mesh of current object
            if (current.activeInHierarchy)
            {
                var renderer = current.GetComponent<MeshRenderer>();
                var skinRenderer = current.GetComponent<SkinnedMeshRenderer>();

                if (skinRenderer != null)
                {
                    Mesh mesh = new Mesh();
                    skinRenderer.BakeMesh(mesh);
                    ExtractMeshes(ctrl, current, mesh, skinRenderer.sharedMaterials, resut);
                }
                else if (renderer != null)
                {
                    ExtractMeshes(ctrl, current, renderer.GetComponent<MeshFilter>().sharedMesh, renderer.sharedMaterials, resut);
                }
            }
        }

        return resut;
    }

    private static void ExtractMeshes(RayliceController ctrl, GameObject gameObj, Mesh mesh, Material[] materials, List<RayliceMesh> outMeshes)
    {
        if (mesh.subMeshCount > 0)
        {
            for (int i = 0; i < mesh.subMeshCount; i++)
            {
                if (mesh.GetTopology(i) != MeshTopology.Triangles)
                {
                    Debug.LogWarning("Mesh is not made of triangles and thus ignored!");
                    continue;
                }

                outMeshes.Add(new RayliceMesh()
                                  {
                                      gameObject =  gameObj,
                                      material = ctrl.GetMappedMaterial(materials[i]),
                                      transform = gameObj.transform.localToWorldMatrix,
                                      mesh = ExtractSubMesh(mesh, mesh.GetTriangles(i))
                                  });
            }
        }
        else
        {
            outMeshes.Add(new RayliceMesh()
                              {
                                  gameObject = gameObj,
                                  material = ctrl.GetMappedMaterial(materials[0]),
                                  transform = gameObj.transform.localToWorldMatrix,
                                  mesh = mesh
                              });
        }
    }

    private static Mesh ExtractSubMesh(Mesh mesh, int[] triangles)
    {
        Mesh result = new Mesh();
        var vertices = mesh.vertices;
        var normals = mesh.normals;
        var uvs = mesh.uv;
        var resultVertices = new Vector3[triangles.Length];
        var resultNormals = new Vector3[triangles.Length];
        Vector2[] resultUv = null;

        if (mesh.uv != null)
            resultUv = new Vector2[triangles.Length];

        for (int i = 0; i < triangles.Length; i++)
        {
            int iVertex = triangles[i];
            resultVertices[i] =  vertices[iVertex];
            resultNormals[i] = normals[iVertex];

            if (resultUv != null)
                resultUv[i] = uvs[iVertex];
        }

        result.vertices = resultVertices;
        result.normals = resultNormals;
        result.uv = resultUv;

        return result;
    }
}

