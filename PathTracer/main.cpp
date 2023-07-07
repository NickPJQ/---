#include "SampleRenderer.h"

// our helper library for window handling
#include "glfWindow/GLFWindow.h"
#include <GL/gl.h>

/*! \namespace osc - Optix Siggraph Course */
namespace osc {

  struct SampleWindow : public GLFCameraWindow
  {
    SampleWindow(const std::string &title,
                 const Model *model,
                 const Camera &camera,
                 const QuadLight &light,
                 const float worldScale)
      : GLFCameraWindow(title,camera.from,camera.at,camera.up,worldScale),
        sample(model,light)
    {
      sample.setCamera(camera);
    }
    
    virtual void render() override
    {
      if (cameraFrame.modified) {
        sample.setCamera(Camera{ cameraFrame.get_from(),
                                 cameraFrame.get_at(),
                                 cameraFrame.get_up() });
        cameraFrame.modified = false;
      }
      sample.render();
    }
    
    virtual void draw() override
    {
      sample.downloadPixels(pixels.data());
      if (fbTexture == 0)
        glGenTextures(1, &fbTexture);
      
      glBindTexture(GL_TEXTURE_2D, fbTexture);
      GLenum texFormat = GL_RGBA;
      GLenum texelType = GL_UNSIGNED_BYTE;
      glTexImage2D(GL_TEXTURE_2D, 0, texFormat, fbSize.x, fbSize.y, 0, GL_RGBA,
                   texelType, pixels.data());

      glDisable(GL_LIGHTING);
      glColor3f(1, 1, 1);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, fbTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      
      glDisable(GL_DEPTH_TEST);

      glViewport(0, 0, fbSize.x, fbSize.y);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.f, (float)fbSize.x, 0.f, (float)fbSize.y, -1.f, 1.f);

      glBegin(GL_QUADS);
      {
        glTexCoord2f(0.f, 0.f);
        glVertex3f(0.f, 0.f, 0.f);
      
        glTexCoord2f(0.f, 1.f);
        glVertex3f(0.f, (float)fbSize.y, 0.f);
      
        glTexCoord2f(1.f, 1.f);
        glVertex3f((float)fbSize.x, (float)fbSize.y, 0.f);
      
        glTexCoord2f(1.f, 0.f);
        glVertex3f((float)fbSize.x, 0.f, 0.f);
      }
      glEnd();
    }
    
    virtual void resize(const vec2i &newSize) 
    {
      fbSize = newSize;
      sample.resize(newSize);
      pixels.resize(newSize.x*newSize.y);
    }

    virtual void key(int key, int mods)
    {
      if (key == 'D' || key == ' ' || key == 'd') {
        sample.denoiserOn = !sample.denoiserOn;
        std::cout << "denoising now " << (sample.denoiserOn?"ON":"OFF") << std::endl;
      }
      if (key == 'A' || key == 'a') {
        sample.accumulate = !sample.accumulate;
        std::cout << "accumulation/progressive refinement now " << (sample.accumulate?"ON":"OFF") << std::endl;
      }
      if (key == ',') {
        sample.launchParams.numPixelSamples
          = std::max(1,sample.launchParams.numPixelSamples-1);
        std::cout << "num samples/pixel now "
                  << sample.launchParams.numPixelSamples << std::endl;
      }
      if (key == '.') {
        sample.launchParams.numPixelSamples
          = std::max(1,sample.launchParams.numPixelSamples+1);
        std::cout << "num samples/pixel now "
                  << sample.launchParams.numPixelSamples << std::endl;
      }
      /* 按'c'添加方块 */
      if (key == 'C' || key == 'c') {
          sample.updateAccel();
      }
      /* 按'l'添加光源 */
      if (key == 'L' || key == 'l') {
          sample.updateLight();
      }
      /*按's'截图*/
      if (key == 's' || key == 'S') {
          std::cout << "截图已保存" << std::endl;
          screenShot();
      }


      /* 相机移动 */
      vec3f leftVector;
      if (key == GLFW_KEY_LEFT) {
          leftVector = cross(cameraFrame.get_up(), cameraFrame.get_at() - cameraFrame.get_from());
          leftVector = leftVector / dot(leftVector, leftVector);
          cameraFrame.position += leftVector * cameraFrame.motionSpeed;
          cameraFrame.modified = true;
      }
      if (key == GLFW_KEY_RIGHT) {
          leftVector = cross(cameraFrame.get_up(), cameraFrame.get_at() - cameraFrame.get_from());
          leftVector = leftVector / dot(leftVector, leftVector);
          cameraFrame.position -= leftVector * cameraFrame.motionSpeed;
          cameraFrame.modified = true;
      }
      if (key == GLFW_KEY_UP) {
          float step = 1e-2 * cameraFrame.motionSpeed;
          const vec3f poi = cameraFrame.getPOI();
          const float minReqDistance = 0.1f * cameraFrame.motionSpeed;
          cameraFrame.poiDistance = max(minReqDistance, cameraFrame.poiDistance - step);
          cameraFrame.position = poi + cameraFrame.poiDistance * cameraFrame.frame.vz;
          cameraFrame.modified = true;
      }
      if (key == GLFW_KEY_DOWN) {
          float step = -(1e-2 * cameraFrame.motionSpeed);
          const vec3f poi = cameraFrame.getPOI();
          const float minReqDistance = 0.1f * cameraFrame.motionSpeed;
          cameraFrame.poiDistance = max(minReqDistance, cameraFrame.poiDistance - step);
          cameraFrame.position = poi + cameraFrame.poiDistance * cameraFrame.frame.vz;
          cameraFrame.modified = true;
      }

    }
    

    vec2i                 fbSize;
    GLuint                fbTexture {0};
    SampleRenderer        sample;
    std::vector<uint32_t> pixels;
  };
  
  
  /*! main entry point to this example - initially optix, print hello
    world, then exit */
  extern "C" int main(int ac, char **av)
  {
    try {
      Model *model = loadOBJ(
#ifdef _WIN32
      // on windows, visual studio creates _two_ levels of build dir
      // (x86/Release)
      "../../models/CornellBox-Original.obj"
#else
      // on linux, common practice is to have ONE level of build dir
      // (say, <project>/build/)...
      "..CornellBox-Original.obj"
#endif
                             );//模型导入
      Camera camera = { /*from*/ vec3f(0.0f, 1.0f, 4.0f),//vec3f(-1293.07f, 154.681f, -0.7304f),
                        /* at */model->bounds.center()-vec3f(0,0, 100),
                        /* up */vec3f(0.f,1.f,0.f) };

      // 光源的创建
      const float light_size = 0.25f;
      QuadLight light = { /* origin */ vec3f(0.0f - light_size, 1.96f, 0.0f - light_size),//vec3f(-1000 - light_size,800,-light_size),
                          /* edge 1 */ vec3f(2.f*light_size,0,0),
                          /* edge 2 */ vec3f(0,0,2.f*light_size),
                          /* power */  vec3f(30000.f),
                          /* color */  vec3f(0.3f, 0.3f, 0.1f)};
                      
      // something approximating the scale of the world, so the
      // camera knows how much to move for any given user interaction:
      const float worldScale = length(model->bounds.span());

      SampleWindow *window = new SampleWindow("Optix 7 Course Example",
                                              model,camera,light,worldScale);//窗口的title
      window->enableFlyMode();
      
      std::cout << "Press 'a' to enable/disable accumulation/progressive refinement" << std::endl;
      std::cout << "Press 'd' to enable/disable denoising" << std::endl;
      std::cout << "Press ',' to reduce the number of paths/pixel" << std::endl;
      std::cout << "Press '.' to increase the number of paths/pixel" << std::endl;
      std::cout << "Press 'c' to add a cube" << std::endl;
      std::cout << "Press 'l' to add a light source" << std::endl;
      std::cout << "Press 's' to get screenshot" << std::endl;
      //window->run();
      window->run_1(&(window->cameraFrame));//显示相机坐标

      
    } catch (std::runtime_error& e) {
      std::cout << GDT_TERMINAL_RED << "FATAL ERROR: " << e.what()
                << GDT_TERMINAL_DEFAULT << std::endl;
	  std::cout << "Can't load the right model! The model may not exist!" << std::endl;
	  exit(1);
    }
    return 0;
  }
  
} // ::osc
