# ffmpegLearn

课程学习笔记:

https://edu.51cto.com/course/24960.html

##### 环境

+ Qt5.15
+ vs2019
+ ffmpeg4.3 手动编译https://blog.csdn.net/ihmhm12345/article/details/109645568

##### 使用

+ clone 到本地后,使用Qt5.15直接打开 ffmpegL.pro 即可
+ 多个Qt子项目,使用mvsc2019 x86版本编译

##### 子项目说明 由简到复杂按以下项目顺序

​		注意看每个main.cpp,开头部分的宏,不同宏会启用不同功能

+ Logger:日志,后续项目大部分都有使用到

+ mergePicture: 合并图片,sdl渲染显示

+ sdlPlayYUV:sdl渲染显示yuv

+ sdlTest:sdl测试

+ testAVFrame : AVFrame测试

+ videoRenderer : 视频渲染接口,大多数子项目会用到

+ avFrame2renderer:AVFrame使用videoRenderer渲染

  ​	在main文件中,有进行不同sleep准确度测试,注释不同宏可切换

+ swsScaleTest:测试SwsScale,所有测试程序都只是在main.cpp中,测试为转换yuv到rgba并保存rgba文件,再由rgba转换到yuv...这里的rgba和yuv后面可以用来测试

+ YuvOrRgbRender:从本项目开始,VideoRerdererView和SDLRerderer进行重构,后续项目会用此项目文件中的VideoRerdererView和SDLRerderer,,已写好videoRenderer.pri..后续直接包含即可,lib库的头文件均已包含,dll库在编译时会自动复制到程序运行目录.

  + 本项目支持播放yuv420p,ARGB,RGBA,ABGR,BGRA,RGB24,BGR24的播放

  + 在播放时,应设置好界面上视频宽高,读取yuv420p或者rgb文件时,是按设置的宽度来读取的

    ffmpeg转换格式的命令:

    ffmpeg.exe -i .\binghe.mp4 -ss 5:00 -t 1:00 -pix_fmt rgba rgba.rgb

    ffmpeg.exe -i .\binghe.mp4 -ss 5:00 -t 1:00 -pix_fmt rgb24 rgba.rgb
  
+ testEncode:测试H264和h265解码,其中包含四个宏,用来控制编码的码率,具体可看里面写的注释

+ encode: 封装编码器, encode线程安全,所有ffmpeg函数全部封装在其中,解决编码过程缓冲未取的问题, main文件为测试文件

+ testDecodec:测试解码,全部程序在main.cpp中,有添加硬解,启用宏即可

+ Codec:重构Encodec,封装Decodec.添加硬解,添加硬解后直接在gpu渲染,直接在gpu渲染只支持dxva2

+ testDemux: 测试解封装,main.cpp..包含两个宏,TEST_PLAY,TEST_REMUX,,,TEST_PLAY指解封装后,播放,TEST_REMUX是解封装后,再封装为mp4文件

