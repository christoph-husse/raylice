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
using UnityEngine;

namespace Raylice
{
    public struct RayliceMesh : IEqualityComparer<RayliceMesh>
    {
        public bool Equals(RayliceMesh a, RayliceMesh b)
        {
            return a.gameObject.Equals(b.gameObject);
        }


        public int GetHashCode(RayliceMesh a)
        {
            return a.gameObject.GetHashCode();
        }

        private const Int32 MESH_MAGIC = 0x76FA0B62;

        public GameObject gameObject;
        public Matrix4x4 transform;
        public UnifiedSettings material;
        public Mesh mesh;
        public long id;

        public void Serialize(BinaryWriter writer)
        {
            var vertices = mesh.vertices;
            var normals = mesh.normals;
            var uv = mesh.uv;

            writer.Write((Int32) vertices.Length);
            writer.Write((bool)(uv != null));
            for (int i = 0; i < vertices.Length; i++)
            {
                writer.WriteVector3(vertices[i]);
                writer.WriteVector3(normals[i]);

                if (uv != null)
                {
                    writer.Write((Single) uv[i].x);
                    writer.Write((Single) uv[i].y);
                }
            }
            writer.Write(MESH_MAGIC);
        }

    }
}
