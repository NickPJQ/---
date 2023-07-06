// ======================================================================== //
// Copyright 2018-2019 Ingo Wald                                            //
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

#include "GLFWindow.h"


/*! \namespace osc - Optix Siggraph Course */
namespace osc {
  using namespace gdt;
  
  static void glfw_error_callback(int error, const char* description)
  {
    fprintf(stderr, "Error: %s\n", description);
  }
  
  GLFWindow::~GLFWindow()
  {
    glfwDestroyWindow(handle);
    glfwTerminate();
  }

  GLFWindow::GLFWindow(const std::string &title)
  {
    glfwSetErrorCallback(glfw_error_callback);
    // glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);
      
    if (!glfwInit())
      exit(EXIT_FAILURE);
      
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
      
    handle = glfwCreateWindow(1200, 800, title.c_str(), NULL, NULL);
    if (!handle) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
      
    glfwSetWindowUserPointer(handle, this);
    glfwMakeContextCurrent(handle);
    glfwSwapInterval( 1 );
  }

  /*! callback for a window resizing event */
  static void glfwindow_reshape_cb(GLFWwindow* window, int width, int height )
  {
    GLFWindow *gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);
    gw->resize(vec2i(width,height));
  // assert(GLFWindow::current);
  //   GLFWindow::current->resize(vec2i(width,height));
  }

  /*! callback for a key press */
  static void glfwindow_key_cb(GLFWwindow *window, int key, int scancode, int action, int mods) 
  {
    GLFWindow *gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);
    if (action == GLFW_PRESS) {
      gw->key(key,mods);
    }
  }

  /*! callback for _moving_ the mouse to a new position */
  static void glfwindow_mouseMotion_cb(GLFWwindow *window, double x, double y) 
  {
    GLFWindow *gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);
    gw->mouseMotion(vec2i((int)x, (int)y));
  }

  /*! callback for pressing _or_ releasing a mouse button*/
  static void glfwindow_mouseButton_cb(GLFWwindow *window, int button, int action, int mods) 
  {
    GLFWindow *gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);
    // double x, y;
    // glfwGetCursorPos(window,&x,&y);
    gw->mouseButton(button,action,mods);
  }
  int nframe = 0;
  double  curtime = 0, lasttime = 0;
  void ShowFrameRate(GLFWwindow* window, CameraFrame* cameraFrame)
  {
      char s[100] = { 0 };
      curtime = glfwGetTime();
      double delta = curtime - lasttime;
      nframe++;
      
      
      if (delta>=1.0) {
          if (cameraFrame != nullptr) {
              vec3f pos = cameraFrame->get_from();
              sprintf(s, "final_project  FPS:%4.2f from:[%.2f, %.2f, %.2f]",
                  double(nframe) / delta, pos.x, pos.y, pos.z);
          }
          else {
              sprintf(s, "final_project  FPS:%4.2f",
                  double(nframe) / delta);
          }
          
          
          //printf("帧率为：%f\n", double(nframe) / delta);
          glfwSetWindowTitle(window, s);
          lasttime = curtime;
          nframe = 0;
      }
  }


  void GLFWindow::run()
  {
    int width, height;
    glfwGetFramebufferSize(handle, &width, &height);
    resize(vec2i(width,height));

    // glfwSetWindowUserPointer(window, GLFWindow::current);
    glfwSetFramebufferSizeCallback(handle, glfwindow_reshape_cb);
    glfwSetMouseButtonCallback(handle, glfwindow_mouseButton_cb);
    glfwSetKeyCallback(handle, glfwindow_key_cb);
    glfwSetCursorPosCallback(handle, glfwindow_mouseMotion_cb);
    
    while (!glfwWindowShouldClose(handle)) {
      render();
      draw();
      ShowFrameRate(handle, nullptr);//显示帧率
      glfwSwapBuffers(handle);
      glfwPollEvents();
    }
  }

  void GLFWindow::run_1(CameraFrame* cameraFrame) {//重写该函数
      int width, height;
      glfwGetFramebufferSize(handle, &width, &height);
      resize(vec2i(width, height));

      // glfwSetWindowUserPointer(window, GLFWindow::current);
      glfwSetFramebufferSizeCallback(handle, glfwindow_reshape_cb);
      glfwSetMouseButtonCallback(handle, glfwindow_mouseButton_cb);
      glfwSetKeyCallback(handle, glfwindow_key_cb);
      glfwSetCursorPosCallback(handle, glfwindow_mouseMotion_cb);

      while (!glfwWindowShouldClose(handle)) {
          render();
          draw();
          ShowFrameRate(handle, cameraFrame);//显示帧率
          glfwSwapBuffers(handle);
          glfwPollEvents();
      }
  }

  // GLFWindow *GLFWindow::current = nullptr;
  
} // ::osc
