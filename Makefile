GPU = 0
CC = g++ 
LDFLAGS = -I. `pkg-config --cflags opencv`
DLIBS	= `pkg-config --libs opencv`

Camera_include = Camera.hpp CameraApi.h CameraDefine.h CameraStatus.h
Camera_src = Camera.cpp

UrServer_include = URrobot.hpp Server.hpp Client.hpp
UrServer_src = URrobot.cpp Server.cpp Client.cpp

ALG_include = VisionArithmetic.hpp Get_config.hpp
ALG_src = VisionArithmetic.cpp Get_config.cpp

PsFun_include = PsFunction.hpp VisionArithmetic.hpp Camera.hpp
PsFun_src =  PsFunction.cpp

ifeq ($(GPU), 1) 
    DEFS = -DTAI_GPU 
else
    DEFS = -DTAI_CPU 
endif

all: libCamera.so libUrServer.so libALG.so libPsFun.so


libCamera.so : 
	g++ -fPIC -shared $(Camera_include) $(Camera_src) -o $@ $(LDFLAGS) $(DLIBS)

libUrServer.so : 
	g++ -fPIC -shared $(UrServer_include) $(UrServer_src) -o $@ $(LDFLAGS) $(DLIBS) -lpthread 

libALG.so : 
	g++ -fPIC -shared $(ALG_include) $(ALG_src) -o $@ $(LDFLAGS) $(DLIBS)

libPsFun.so : 
	g++ -fPIC -shared $(PsFun_include) $(PsFun_src) -o $@ $(LDFLAGS) $(DLIBS) $(DEFS)

	
	
clean:
	rm libCamera.so libUrServer.so libALG.so libPsFun.so
