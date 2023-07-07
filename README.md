# Building the Code

This code was intentionally written with minimal dependencies,
requiring only CMake (as a build system), your favorite
compiler (tested with Visual Studio 2017 and 2019 under Windows, and GCC under
Linux), and the OptiX 7 SDK (including CUDA 10.1 and NVIDIA driver recent
enough to support OptiX)

## Dependencies

- a compiler
    - On Windows, tested with Visual Studio 2017 and 2019 community editions
    - On Linux, tested with Ubuntu 18 and Ubuntu 19 default gcc installs
- CUDA 10.1
    - Download from developer.nvidia.com
    - on Linux, suggest to put `/usr/local/cuda/bin` into your `PATH`
- latest NVIDIA developer driver that comes with the SDK
    - download from http://developer.nvidia.com/optix and click "Get OptiX"
- OptiX 7 SDK
    - download from http://developer.nvidia.com/optix and click "Get OptiX"
    - on linux, set the environment variable `OptiX_INSTALL_DIR` to wherever you installed the SDK.
    `export OptiX_INSTALL_DIR=<wherever you installed OptiX 7 SDK>`
    - on windows, the installer should automatically put it into the right directory

The only *external* library we use is GLFW for windowing, and
even this one we build on the fly under Windows, so installing
it is required only under Linux. 

Detailed steps below:
## 环境配制
- VS2017 + Optix 7.7.0 SDK + CUDA 12.2
- 
## Building under Windows

- Install Required Packages
	- see above: CUDA 10.1, OptiX 7 SDK, latest driver, and cmake
- download or clone the source repository
- Open `CMake GUI` from your start menu
	- point "source directory" to the downloaded source directory
	- point "build directory" to <source directory>/build (agree to create this directory when prompted)
	- click 'configure', then specify the generator as Visual Studio 2017 or 2019, and the Optional platform as x64. If CUDA, SDK, and compiler are all properly installed this should enable the 'generate' button. If not, make sure all dependencies are properly installed, "clear cache", and re-configure.
	- click 'generate' (this creates a Visual Studio project and solutions)
	- click 'open project' (this should open the project in Visual Studio)

## 基本原理
- 基于渲染方程实现光线追踪，本程序在计算间接光照时只考虑了来自物体表面上半球的光线，即反射部分。
- 符号说明：v&ω0表示观察方向向量，n表示法向量，l表示光线方向向量，α表示粗糙度，h为l和v的半程向量。
- 反射部分的计算方程如下：
- ![e2a30cbb3463e82573fcce14828ee8b](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/b2a942f2-f3b8-41b6-8766-e3cc7ca2718e)
- 其中的积分我们用黎曼和予以近似，Li为间接光照的辐照度，fr为BRDF。
- 本程序使用的是Cook-Torrance BRDF，其计算公式为：
- ![060a2c0600ecd878eeb90d279b65d44](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/32e3a45f-3138-4257-be61-70093d9ddbaa)
- 其中kd和ks都可以直接从mtl文件中读取出来。
- 剩余的各个部分的计算方式为：
- ![d35ba4c84eb8aebdc3c97a73e6a42f0](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/2f83fa87-c5ef-41e1-a5e3-26157811727e)，其中的c为color
- ![5995934bc46dd021fab3724c8cdb681](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/f02f6306-9122-44e0-bc1c-e02fec97a940)
- D为![b7b2c24d8834bbe3c79d0b88de2e208](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/ddf4700f-c0d6-477d-bc5f-aed2a622aaaf)
- G为![1809085235dbfd49e297a78f23bdfa3](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/aa0aad65-9aa1-442d-bf75-ea6002269152)
- 其中的Gsub为![cf0230773d44883ed58b830e2675265](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/b1a464c2-4ced-4e2d-af54-28a29111afe9)
- Gsub中的k为![64dfed4c1ca8ad761b8391520726dd8](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/31091618-064d-4fac-8dc4-54ac41224a9b)
- F为![3d250c7c38ac26e9f842469ff38b0e0](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/9d2c0551-de7b-4107-a9a3-405e466befa2)
- 最终我们得到展开公式![caba5e751487dd2d8b97a048c4e96be](https://github.com/NickPJQ/Graphics-RayTracing/assets/104704254/bd268c8a-b37a-40bf-b87d-36b51ac4c30d)
- 这样我们就实现了对简介光照的计算。

## 交互方式&实现方式
- 软阴影计算：
    - 实现方式：
        - 在每个光源的范围内随机采样，并调用optixtrace（）计算像素点到光源之间是否有遮罩，如果没有就加上这次采样的颜色，最后除以采样总数。
- 光线追踪：
    - 实现方式：
        - 每次采样从相机向目标像素点发射一条光线，这条光线若无交点则结束对这条光线的追踪，否则在辐照度中加上直接光照，在衰减上乘以相应系数，像素点采样的总光照中加上衰减*辐照度，然后再以该像素点位置和对该像素点采样的方向继续追踪，直至反弹次数超过上限或者俄罗斯轮盘赌结束对这条光线的追踪。
- 按'C'在场景中添加方块
    - 需要输入方块的中心坐标，边长和颜色
    - 实现方式：
        - 预先设定最大可添加方块数量MAX_CUBE_NUM，默认为16，在buildAccel函数建立Acceleration Structure时新增MAX_CUBE_NUM个build input，顶点坐标都设置为0使其不可见，在buildflag中添加         OPTIX_BUILD_FLAG_ALLOW_UPDATE来允许后续对Acceleration Structure的更新；在buildSBT函数中同样新增MAX_CUBE_NUM * RAY_TYPE_COUNT个HitgroupRecord
        - 然后添加函数updateAccel，这个函数首先接受输入的方块中心坐标，边长和颜色，然后以坐标和边长建立方块的TriangleMesh，更新对应的buffer和buildinput，然后调用updateSBT函数更新方块颜色，最后将OptixAccelBuildOptions::operation设置为OPTIX_BUILD_OPERATION_UPDATE后调用optixBuildAccel完成更新
        - 添加函数updateSBT来更新对应方块的HitgroupRecord中的color，然后上传到sbt中

- 按'L'在场景中添加长方形光源
    - 需要输入光源某顶点的坐标及其相邻两边的方向、光源的强度和颜色
    - 实现方式：
        - 预先设定最大光源数量MAX_LIGHT_NUM，默认为8，在LaunchParams中将原先的一个struct light改为struct light[MAX_LIGHT_NUM]数组，再添加一个整数lightNum记录当前光源数量
        - 添加函数updateLight接受输入并修改launchParams.light

- 按'D'开关降噪
- 按','减少每个像素的采样数
- 按'.'增加每个像素的采样数

- 使用方向键控制相机位置移动

- 按'S'键对当前渲染结果截图，保存在./Screenshot中
	- 实现方式：
 		- 将渲染出的每一个像素RGB值保存起来，按bmp位图格式写入目标文件   
- 在窗口标题栏显示FPS和当前相机坐标
	- 实现方式：
		- 每次渲染新的帧时计数，在一定时间间隔（1s）后更新FPS值
	   	- 使用glfWINDOW的cameraFrame类维护相机坐标，在每次渲染时更新 
