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

namespace Raylice
{
    public static class IOExtensions
    {
        private static readonly byte[] buffer = new byte[1024];
        private static readonly System.Security.Cryptography.MD5 md5 = System.Security.Cryptography.MD5.Create();

        public static void WriteTexture2D(this BinaryWriter writer, Texture2D texture)
        {
            if (texture != null)
            {
                var path = AssetDatabase.GetAssetPath(texture);
                var asset = AssetImporter.GetAtPath(path);

                if (asset is TextureImporter)
                {
                    var ti = (TextureImporter)asset;

                    if (!ti.isReadable || (ti.npotScale != TextureImporterNPOTScale.None) || (ti.maxTextureSize != 2048))
                    {
                        ti.textureFormat = TextureImporterFormat.AutomaticTruecolor;
                        ti.npotScale = TextureImporterNPOTScale.None;
                        ti.maxTextureSize = 2048;
                        ti.isReadable = true;
                        ti.mipmapEnabled = true;
                        AssetDatabase.ImportAsset(path);
                    }
                }
            }

            int channelCount;
            switch (texture.format)
            {
                case TextureFormat.Alpha8:
                    channelCount = 1;
                    break;

                case TextureFormat.ARGB32:
                case TextureFormat.BGRA32:
                case TextureFormat.DXT1:
                case TextureFormat.DXT5:
                case TextureFormat.RGBA4444:
                case TextureFormat.RGBA32:
                case TextureFormat.ARGB4444:
                    channelCount = 4;
                    break;

                case TextureFormat.RGB565:
                case TextureFormat.RGB24:
                    channelCount = 3;
                    break;

                default:
                    throw new ArgumentException("Texture \"" + texture.name + "\" has an unsupported format " + texture.format + "!");
            }

            var pixels = texture.GetPixels32();
            byte[] pixelBytes = new byte[pixels.Length * channelCount];

            for (int i = 0, j = 0; i < pixels.Length; i++)
            {
                if (channelCount == 4)
                {
                    pixelBytes[j++] = pixels[i].r;
                    pixelBytes[j++] = pixels[i].g;
                    pixelBytes[j++] = pixels[i].b;
                    pixelBytes[j++] = pixels[i].a;
                }
                else if (channelCount == 3)
                {
                    pixelBytes[j++] = pixels[i].r;
                    pixelBytes[j++] = pixels[i].g;
                    pixelBytes[j++] = pixels[i].b;
                }
                else
                {
                    pixelBytes[j++] = pixels[i].a;
                }
            }

            writer.WriteNativeString(texture.name);
            writer.Write((Int32)channelCount);
            writer.Write((Int32)texture.width);
            writer.Write((Int32)texture.height);
            writer.Write((Int32)pixelBytes.Length);
            writer.Write(pixelBytes, 0, pixelBytes.Length);
        }

        public static void WriteColor(this BinaryWriter writer, Color color)
        {
            writer.Write((Single)color.a);
            writer.Write((Single)color.r);
            writer.Write((Single)color.g);
            writer.Write((Single)color.b);
        }

        public static Color ReadColor(this BinaryReader reader)
        {
            float a = reader.ReadSingle();
            float r = reader.ReadSingle();
            float g = reader.ReadSingle();
            float b = reader.ReadSingle();
            return new Color(r, g, b, a);
        }


        public static long ComputeHash(this Stream stream, long start, long count)
        {
            long backupPos = stream.Position;
            try
            {
                int bytesRead = -1;
                md5.Initialize();

                stream.Position = start;

                while ((count > 0) && (bytesRead != 0))
                {
                    bytesRead = stream.Read(buffer, 0, (int)Math.Min(count, buffer.Length));
                    md5.TransformBlock(buffer, 0, bytesRead, null, 0);
                    count -= bytesRead;
                }

                md5.TransformFinalBlock(new byte[0], 0, 0);

                var hashCode = md5.Hash;
                long result = 0;

                for (int i = 0; i < hashCode.Length; i++)
                {
                    result ^= ((long)hashCode[i]) << ((i % 8) * 8);
                }

                return result;
            }
            finally
            {
                stream.Position = backupPos;
            }
        }


        public static void WriteVector3(this BinaryWriter writer, Vector3 vector)
        {
            writer.Write((Single)vector.x);
            writer.Write((Single)vector.y);
            writer.Write((Single)vector.z);
        }

        public static Vector3 ReadVector3(this BinaryReader reader)
        {
            float x = reader.ReadSingle();
            float y = reader.ReadSingle();
            float z = reader.ReadSingle();
            return new Vector3(x, y, z);
        }

        public static void WriteQuaternion(this BinaryWriter writer, Quaternion quat)
        {
            writer.Write((Single)quat.x);
            writer.Write((Single)quat.y);
            writer.Write((Single)quat.z);
            writer.Write((Single)quat.w);
        }

        public static Quaternion ReadQuaternion(this BinaryReader reader)
        {
            float x = reader.ReadSingle();
            float y = reader.ReadSingle();
            float z = reader.ReadSingle();
            float w = reader.ReadSingle();
            return new Quaternion(x, y, z, w);
        }

        public static void WriteNativeString(this BinaryWriter writer, String str)
        {
            var bytes = Encoding.UTF8.GetBytes(str);
            writer.Write((Int32)bytes.Length);
            writer.Write(bytes, 0, bytes.Length);
        }

        public static String ReadNativeString(this BinaryReader reader)
        {
            int byteCount = reader.ReadInt32();
            return Encoding.UTF8.GetString(reader.ReadBytes(byteCount));
        }

        public static void WriteMatrix(this BinaryWriter writer, Matrix4x4 mat)
        {
            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 4; row++)
                {
                    writer.Write((Single)mat[row, col]);
                }
            }
        }

        public static Matrix4x4 ReadMatrix(this BinaryReader reader)
        {
            Matrix4x4 mat = new Matrix4x4();

            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 4; row++)
                {
                    mat[row, col] = reader.ReadSingle();
                }
            }

            return mat;
        }
    }
}
