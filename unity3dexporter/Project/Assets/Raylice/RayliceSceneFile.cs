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
    public class RayliceSceneFile : IDisposable
    {
        private const Int64 SYNC_MAGIC = 0x72BCC3123489D6EA;
        private readonly Dictionary<RayliceMesh, Int64> meshToId = new Dictionary<RayliceMesh, long>(new RayliceMesh());
        private readonly Dictionary<Texture2D, Int64> texToId = new Dictionary<Texture2D, long>();
        private readonly List<RayliceMesh> activeMeshes = new List<RayliceMesh>();
        private readonly Dictionary<Int64, Int64> texIdToOffset = new Dictionary<Int64, Int64>();
        private readonly Dictionary<Int64, Int64> meshIdToOffset = new Dictionary<Int64, Int64>();
        private String fileName;
        private FileStream fileStream;
        private UnifiedSettings settings;

        public String FileName { get { return fileName; } }
        public Int64 MeshCount { get; private set; }
        public Int64 UpdatedMeshCount { get; private set; }
        public Int64 MeshSizeInBytes { get; private set; }
        public Int64 TextureCount { get; private set; }
        public Int64 UpdatedTextureCount { get; private set; }
        public Int64 TextureSizeInBytes { get; private set; }
        public Int64 TotalSizeInBytes { get; private set; }

        public void Dispose()
        {
            Close();
        }

        public void Close()
        {
            if(fileStream != null)
                fileStream.Dispose();

            fileStream = null;
        }

        public void Open(String fileName)
        {
            try
            {
                fileStream = new FileStream(fileName, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.Read);
                this.fileName = Path.GetFullPath(fileName);
            }
            catch (Exception e)
            {
                throw new ArgumentException("Scene file \"" + fileName + "\" could not be opened.", e);
            }

            Load();
        }

        public void Clear()
        {
            if (fileStream != null)
                fileStream.SetLength(0);
        }

        public void Load()
        {
            try
            {
                BinaryReader reader = new BinaryReader(fileStream);
                long basePosition = 0;
                List<UnifiedSettings> activeMaterials = new List<UnifiedSettings>();


                MeshCount = 0;
                UpdatedMeshCount = 0;
                MeshSizeInBytes = 0;
                TextureCount = 0;
                UpdatedTextureCount = 0;
                TextureSizeInBytes = 0;
                TotalSizeInBytes = 0;

                activeMeshes.Clear();
                meshIdToOffset.Clear();
                texIdToOffset.Clear();

                settings = new UnifiedSettings();

                if (fileStream.Length > 0)
                {
                    // read last Int64 to get offset of first byte of metadata
                    fileStream.Position = fileStream.Length - 8;
                    basePosition = fileStream.Position = reader.ReadInt64();

                    // deserialize global config
                    ReadSync(reader);
                    settings = UnifiedSettings.DeserializeFormat(reader);
                    settings.DeserializeData(reader);

                    // deserialize texture dictionary
                    ReadSync(reader);
                    int texCount = reader.ReadInt32();
                    for (int i = 0; i < texCount; i++)
                    {
                        Int64 id = reader.ReadInt64();
                        Int64 offset = reader.ReadInt64();

                        texIdToOffset.Add(id, offset);
                    }

                    // deserialize materials
                    ReadSync(reader);
                    int matCount = reader.ReadInt32();
                    var matFormat = settings.GetChildFormat("Material");
                    for (int i = 0; i < matCount; i++)
                    {
                        var matSettings = matFormat.Clone();
                        matSettings.DeserializeData(reader);

                        foreach(var tex in matSettings.EnumTextures())
                        {
                            var unused = texIdToOffset[tex.id];
                            tex.SetTexture2D(new Texture2D(1,1,TextureFormat.RGBA32, false));
                        }

                        activeMaterials.Add(matSettings);
                    }

                    // deserialize mesh dictionary
                    ReadSync(reader);
                    int meshCount = reader.ReadInt32();
                    for (int i = 0; i < meshCount; i++)
                    {
                        Int64 id = reader.ReadInt64();
                        Int64 offset = reader.ReadInt64();

                        meshIdToOffset.Add(id, offset);
                    }

                    // deserialize active meshes
                    ReadSync(reader);
                    meshCount = reader.ReadInt32();
                    for (int i = 0; i < meshCount; i++)
                    {
                        Int64 id = reader.ReadInt64();
                        Int32 matIndex = reader.ReadInt32();
                        Matrix4x4 transform = reader.ReadMatrix();
                        activeMeshes.Add(new RayliceMesh()
                                             {
                                                 id = id,
                                                 transform = transform,
                                                 material = activeMaterials[matIndex]
                                             });
                    }

                    ReadSync(reader);
                }

                // remove non-existing meshes from cache
                foreach (var e in meshToId.ToArray())
                {
                    if (!meshIdToOffset.ContainsKey(e.Value))
                        meshToId.Remove(e.Key);
                }

                // remove non-existing textures from cache
                foreach (var e in texToId.ToArray())
                {
                    if (!texIdToOffset.ContainsKey(e.Value))
                        texToId.Remove(e.Key);
                }

                fileStream.Position = basePosition;
            }
            catch (Exception e)
            {

                throw new ArgumentException("Scene file at \"" + FileName + "\" could not be parsed.", e);
            }
        }

        private void ReadSync(BinaryReader reader)
        {
            if(reader.ReadInt64() != SYNC_MAGIC)
                throw new ArgumentException("Data stream is out of sync!");
        }

        private void WriteSync(BinaryWriter writer)
        {
            writer.Write((Int64)SYNC_MAGIC);
        }

        public void WriteScene(UnifiedSettings settings, IEnumerable<RayliceMesh> meshes)
        {
            Load();

            MeshCount = 0;
            UpdatedMeshCount = 0;
            MeshSizeInBytes = 0;
            TextureCount = 0;
            UpdatedTextureCount = 0;
            TextureSizeInBytes = 0;
            TotalSizeInBytes = 0;

            BinaryWriter writer = new BinaryWriter(fileStream);

            // write meshes
            activeMeshes.Clear();
            foreach (var inMesh in meshes)
            {
                var mesh = inMesh;

                if (!meshToId.TryGetValue(mesh, out mesh.id))
                {
                    // add mesh to file
                    long fileOffset = fileStream.Position;

                    WriteSync(writer);
                    mesh.Serialize(writer);
                    mesh.id = fileStream.ComputeHash(fileOffset, fileStream.Position - fileOffset);

                    // check for duplicates
                    if (meshIdToOffset.ContainsKey(mesh.id))
                    {
                        // revert changes
                        fileStream.SetLength(fileOffset);
                    }
                    else
                    {
                        UpdatedMeshCount++;
                        meshIdToOffset.Add(mesh.id, fileOffset);
                    }

                    meshToId.Add(mesh, mesh.id);
                }

                activeMeshes.Add(mesh);
            }

            MeshCount = activeMeshes.Count;

            // write textures
            foreach (var mesh in activeMeshes)
            {
                foreach (var tex in mesh.material.EnumTextures())
                {
                    long id;
                    if (!texToId.TryGetValue(tex.GetTexture2D(), out id))
                    {
                        // add texture to file
                        long fileOffset = fileStream.Position;

                        WriteSync(writer);
                        writer.WriteTexture2D(tex.GetTexture2D());
                        id = fileStream.ComputeHash(fileOffset, fileStream.Position);

                        // check for duplicates
                        if (texIdToOffset.ContainsKey(id))
                        {
                            // revert changes
                            fileStream.SetLength(fileOffset);
                        }
                        else
                        {
                            UpdatedTextureCount++;
                            texIdToOffset.Add(id, fileOffset);
                        }

                        tex.id = id;
                        texToId.Add(tex.GetTexture2D(), id);
                    }
                    else
                        tex.id = id;
                }
            }

            TextureCount = texIdToOffset.Count;

            // write metadata
            long metaStart = fileStream.Position;

            WriteSync(writer);
            settings.SerializeFormat(writer);
            settings.SerializeData(writer);

            WriteSync(writer);
            writer.Write((Int32)texIdToOffset.Count);
            foreach (var tex in texIdToOffset)
            {
                writer.Write((Int64)tex.Key);
                writer.Write((Int64)tex.Value);
            }

            WriteSync(writer);
            writer.Write((Int32)activeMeshes.Count);
            foreach(var mesh in activeMeshes)
            {
                mesh.material.SerializeData(writer);
            }

            WriteSync(writer);
            writer.Write((Int32)meshIdToOffset.Count);
            foreach (var mesh in meshIdToOffset)
            {
                writer.Write((Int64)mesh.Key);
                writer.Write((Int64)mesh.Value);
            }

            WriteSync(writer);
            writer.Write((Int32)activeMeshes.Count);
            int iMatIndex = 0;
            foreach (var mesh in activeMeshes)
            {
                writer.Write((Int64)mesh.id);
                writer.Write((Int32)iMatIndex);
                writer.WriteMatrix(mesh.transform);

                iMatIndex++;
            }

            WriteSync(writer);
            writer.Write((Int64)metaStart);

            fileStream.Flush();
            TotalSizeInBytes = fileStream.Length;
            fileStream.Close();
        }

        public IEnumerable<RayliceMesh> ReadGeometry()
        {
            Load();

            return activeMeshes;
        }

        public UnifiedSettings ReadSettings()
        {
            Load();

            return settings;
        }
    }
}
