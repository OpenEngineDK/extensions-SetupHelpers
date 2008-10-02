ADD_LIBRARY(Extensions_SetupHelpers
  Utils/SimpleSetup.h
  Utils/SimpleSetup.cpp
)

TARGET_LINK_LIBRARIES(Extensions_SetupHelpers
  # core libraries
  OpenEngine_Core
  OpenEngine_Devices
  OpenEngine_Display
  OpenEngine_Logging
  OpenEngine_Renderers
  OpenEngine_Resources
  OpenEngine_Scene
  # extenstions libraries
  Extensions_AccelerationStructures
  Extensions_ColladaResource
  Extensions_OBJResource
  Extensions_OpenGLRenderer
  Extensions_SDL
  Extensions_TGAResource
  Extensions_HUD
)
