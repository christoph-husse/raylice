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


#ifndef _OPENGL_WINDOW_H_
#define _OPENGL_WINDOW_H_

#include "stdafx.h"



class OpenGLWindow
{
private:
	std::atomic_bool isClosed, isRunning;
	int handle, width, height;
	std::thread thread; // seems like GLUT is not (even) capable of multi-threading, so still only one window at a time... Iiish.
	Vector3 mouseXY;
	bool isMouseDown, isLeftButtonDown, isInitialized;
	static OpenGLWindow* openWindow;

	static void OnResizeStatic(int w, int h);
	static void OnRenderStatic();
	static void OnMouseDragStatic(int x, int y);
	static void OnMouseMoveStatic(int x, int y);
	static void OnUpdateStatic(int unused);
	static void OnMouseDownStatic(int button, int state, int x, int y);
	static void OnKeyDownStatic(unsigned char key, int x, int y);

	static OpenGLWindow* getCurrent() { return openWindow; }

protected:
	virtual void OnResize() { }
	virtual void OnInitialize() { }
	virtual void OnKeyDown(unsigned char key) { }
	virtual void OnRender() = 0;

public:

	OpenGLWindow(std::string title, std::vector<char*> cmdArgs, int wndWidth = 640, int wndHeight = 480);
	virtual ~OpenGLWindow() { WaitUntilClosed(); }

	int ReadPixelAtMouse() const;
	bool IsClosed() const;
	void Close();
	void WaitUntilClosed();
	Vector3 GetMouseXY();
	bool IsMouseDown();
	bool IsLeftButtonDown();
};

#endif