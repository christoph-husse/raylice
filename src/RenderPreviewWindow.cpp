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


#include "stdafx.h"


void RenderPreviewWindow::OnRender()
{
	if(watch.GetElapsedSeconds().count() > 1)
	{
		watch.Reset();
		auto& fb = tracer.GetFrameBuffer();
		fb.BlitToOpenGL(Rect(0, 0, fb.GetWidth(), fb.GetHeight()));
	}

	Pixel clearColor = tracer.GetClearColor();
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, textureId);
	glColor3d(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0,0.0); glVertex3d(-1.0,1.0,0);
	glTexCoord2d(1.0,0.0); glVertex3d(1.0,1.0,0);
	glTexCoord2d(1.0,1.0); glVertex3d(1.0,-1.0,0);
	glTexCoord2d(0.0,1.0); glVertex3d(-1.0,-1.0,0);
	glEnd();
}

void RenderPreviewWindow::OnKeyDown(unsigned char key)
{

}

void RenderPreviewWindow::OnInitialize()
{
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	std::vector<unsigned char> pixels(tracer.GetWidth() * tracer.GetHeight() * 3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tracer.GetWidth(), tracer.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
}

std::pair<int, int> RenderPreviewWindow::GetSize(int width, int height)
{
	float aspect = width / (float)height;

	if(aspect > 1)
	{
		width = std::min(1400, width);
		height = (int)(width / aspect);
	}
	else
	{
		height = std::min(1400, height);
		width = (int)(height * aspect);
	}

	return std::make_pair(width, height);
}


RenderPreviewWindow::~RenderPreviewWindow()
{
	WaitUntilClosed();
}
	
RenderPreviewWindow::RenderPreviewWindow(RayTracer& tracer) : 
	OpenGLWindow("RayTracer - Rendering Preview", std::vector<char*>(), GetSize(tracer.GetWidth(), tracer.GetHeight()).first, GetSize(tracer.GetWidth(), tracer.GetHeight()).second), 
	tracer(tracer),
	textureId(-1)
{
}
