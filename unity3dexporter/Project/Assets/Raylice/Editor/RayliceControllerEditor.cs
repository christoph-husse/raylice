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

[CustomEditor(typeof(RayliceController))]
public class RayliceControllerEditor : Editor
{
    private class SelectiveRender
    {
        private Vector3 dragStart;
        private bool isBeingSetup = true;
        private bool isViewing;
        private bool isDragging;

        public CameraParameters Camera; 
        public bool IsVisible { get { return isViewing; } }

        public SelectiveRender()
        {
            HandleUtility.Repaint(); 
        }

        public void Show()
        {
            isViewing = true;
            HandleUtility.Repaint(); 
        }

        public void Hide()
        {
            isViewing = false;
            HandleUtility.Repaint(); 
        }

        public void OnSceneGUI()
        {
            var current = Event.current;

            if ((current.type == EventType.MouseDown) && (current.button == 1) && isBeingSetup)
            {
                Camera = new CameraParameters(UnityEngine.Camera.current);
                dragStart = current.mousePosition;
                isDragging = false;
            }

            if ((current.type == EventType.MouseDrag) && (current.button == 1) && isBeingSetup)
            {
                current.Use();
                var xy = current.mousePosition;
                Camera.CameraRect = new Rect(dragStart.x, dragStart.y, xy.x - dragStart.x, xy.y - dragStart.y);
                isDragging = true;
            }

            if ((current.type == EventType.MouseUp) && (current.button == 1) || !isBeingSetup)
            {
                isBeingSetup = false;
                isDragging = false;
            }

            if (isDragging || isViewing)
            {
                var r = Camera.CameraRect = new Rect(Camera.CameraRect.x, Camera.CameraRect.y, Math.Max(50, Camera.CameraRect.width), Math.Max(50, Camera.CameraRect.height));

                if (current.type != EventType.Repaint)
                    current.Use();

                Handles.ClearCamera(new Rect(0, 0, Camera.ScreenSize.x, r.yMin), UnityEngine.Camera.current);
                Handles.ClearCamera(new Rect(0, r.yMin, r.xMin, Camera.ScreenSize.y), UnityEngine.Camera.current);
                Handles.ClearCamera(new Rect(r.xMin, r.yMax, Camera.ScreenSize.x, Camera.ScreenSize.y), UnityEngine.Camera.current);
                Handles.ClearCamera(new Rect(r.xMax, r.yMin, Camera.ScreenSize.x, r.yMax), UnityEngine.Camera.current);

                UnityEngine.Camera.current.rect = new Rect(0.0f, 0.0f, 1f, 1f);
            }
        }
    }

    private struct CameraParameters
    {
        public Matrix4x4 ProjectionMatrix;
        public Matrix4x4 CameraToWorldMatrix;
        public float AspectRatio;
        public Rect CameraRect;
        public Vector3 ScreenSize;

        public CameraParameters(Camera camera)
        {
            ScreenSize = camera.ViewportToScreenPoint(new Vector3(1, 1, 0));
            AspectRatio = ScreenSize.x / ScreenSize.y;
            ProjectionMatrix = camera.projectionMatrix;
            CameraToWorldMatrix = camera.worldToCameraMatrix;
            CameraRect = camera.rect;
        }
    }

  //  private static SelectiveRender selectiveRender;
    private static RayliceSceneFile sceneFile;
    private static SelectiveRender selectiveRender;
    private static CameraParameters sceneCamera = new CameraParameters();

    public void OnSceneGUI()
    {
        sceneCamera = new CameraParameters(Camera.current);

        if(selectiveRender != null)
            selectiveRender.OnSceneGUI();

        if (GUI.changed)
            EditorUtility.SetDirty(target);
    }

    private void AddCameraToSettings(UnifiedSettings settings, CameraParameters camera, string name)
    {
        var cam = settings.AddChildInstance("Camera");
        cam.SetMatrix("Projection", camera.ProjectionMatrix);
        cam.SetMatrix("LocalToWorld", camera.CameraToWorldMatrix);
        cam.SetString("Name", name);
        cam.SetSingle("Aspect", camera.AspectRatio);
        cam.SetVector3("RectOrigin", new Vector3(camera.CameraRect.xMin, camera.CameraRect.yMin));
        cam.SetVector3("RectSize", new Vector3(camera.CameraRect.width, camera.CameraRect.height)); 
    }

    public override void OnInspectorGUI()
    {
        RayliceController mapper = target as RayliceController;
        
        // select Raylice shader
        if(GUILayout.Button("Save Difference"))
        {
            if (sceneFile == null)
                sceneFile = new RayliceSceneFile();

            List<RayliceMesh> meshes = RayliceSceneUtility.CollectGeometry(mapper);
            var format = RayliceController.LoadConfigFormat();

            var settings = format.Clone();

            // collect cameras active in scene
            UnifiedSettings cam;

            foreach (Camera camera in FindObjectsOfType(typeof(Camera)))
            {
                AddCameraToSettings(settings, new CameraParameters(camera), camera.name);
            }

            // add selective render camera
            if (selectiveRender != null)
                AddCameraToSettings(settings, selectiveRender.Camera, "@SelectiveRendering");

            // add scene camera
            AddCameraToSettings(settings, sceneCamera, "@SceneCamera");
             
            // save scene file
            try
            {
                sceneFile.Open("test.scene.raylice");
                sceneFile.WriteScene(settings, meshes);
            }
            finally
            {
                sceneFile.Close();   
            }

            Debug.Log("Scene has been serialized to file \"" + sceneFile.FileName + "\".");
            Debug.Log("    > Meshes:     " + sceneFile.MeshCount + "/" + sceneFile.UpdatedMeshCount + " (total / updated)");
            Debug.Log("    > Textures:   " + sceneFile.TextureCount + " / " + sceneFile.UpdatedTextureCount + " (total / updated)");
            Debug.Log("    > Total size: " + (sceneFile.TotalSizeInBytes / 1000000) + " MB");
        }

        if (GUILayout.Button("Setup Area For Selective Rendering"))
        {
            selectiveRender = new SelectiveRender();
            Debug.Log("Entered selective render setup. To proceed, click on the view screen with the right mouse button and drag it until you selected the desired region you want to render.");
        }

        if ((selectiveRender != null) && !selectiveRender.IsVisible && GUILayout.Button("Show Selective Rendering Area"))
            selectiveRender.Show();

        if ((selectiveRender != null) && selectiveRender.IsVisible && GUILayout.Button("Hide Selective Rendering Area"))
            selectiveRender.Hide();

        if ((selectiveRender != null) && GUILayout.Button("Disable Selective Rendering Area"))
            selectiveRender = null;

        if ((selectiveRender != null) && GUILayout.Button("Render Selected Area"))
        {
        }

        // configure mapping
        var cfgFormat = RayliceController.LoadConfigFormat();

        if (GUI.changed)
            EditorUtility.SetDirty(target);
    }

}