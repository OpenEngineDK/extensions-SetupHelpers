// Simple setup of a graphics engine in OpenEngine.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_SIMPLE_SETUP_H_
#define _OE_SIMPLE_SETUP_H_

//#include <Meta/Config.h>

// include the main interfaces
#include <Core/IEngine.h>
#include <Core/Engine.h>
#include <Devices/IJoystick.h>
#include <Devices/IKeyboard.h>
#include <Devices/IMouse.h>
#include <Display/Camera.h>
#include <Renderers/IRenderer.h>
#include <Renderers/IRenderingView.h>
#include <Scene/ISceneNode.h>
#include <Resources/IModelResource.h>
#include <Display/HUD.h>

// include all the classes that depend on serialization
#include <Resources/SDLImage.h>
// @todo: what about all the scene nodes?

// other std c++ includes
#include <string>

// forward declarations
namespace OpenEngine {
    namespace Core {
        class Engine;
    }
    namespace Display {
        class IEnvironment;
        class IFrame;
        class IRenderCanvas;
        class IViewingVolume;
        class Frustum;
        class HUD;
    }
    namespace Devices {
        class IMouse;
        class IKeyboard;
        class IJoystick;
    }
    namespace Scene {
        class SceneNode;
    }
    namespace Renderers {
        class TextureLoader;
        namespace OpenGL {
            class Renderer;
            class RenderingView;
            class LightRenderer;
            class ShaderLoader;
        }
    }
    namespace Logging {
        class ILogger;
    }
}

namespace OpenEngine {
namespace Utils {

/**
 * The purpose of simple setup is to provide a fairly basic setup of
 * an OpenEngine graphics engine.
 * The interface should be as stable as possible so that it can act
 * as a barrier between core and extension changes reducing the amount
 * of code breakage.
 * It is not meant to solve all setups. More advanced products may
 * very well need to revert to setting up the entire engine from
 * scratch or they may be based on a more advanced setup wrapper.
 * 
 * Using SimpleSetup as a base will allow a project to simply link to
 * this extension alone. SimpleSetup will link to all other required
 * components. What about namespace stuff?
 *
 * This example interface is based on what appears in all the current
 * "projects" built on top of OpenEngine.
 *
 * @code
 * // How to start an empty screen with a 800x600 frame.
 * #include <Utils/SimpleSetup.h>
 * using OpenEngine::Utils::SimpleSetup;
 * int main() {
 *   SimpleSetup setup;
 *   setup.GetEngine().Start();
 *   return 0;
 * }
 * @endcode
 */
class SimpleSetup {
public:

    SimpleSetup(std::string title, 
                Display::IEnvironment* env=NULL, 
                Renderers::IRenderingView* rv=NULL,
                Core::IEngine* eng=NULL,
                Renderers::IRenderer* rend=NULL);

    Core::IEngine& GetEngine() const;
    Display::IFrame& GetFrame() const;
    Display::IRenderCanvas* GetCanvas() const;
    Renderers::IRenderer& GetRenderer() const;
    
    Devices::IMouse&    GetMouse() const;
    Devices::IKeyboard& GetKeyboard() const;
    Devices::IJoystick& GetJoystick() const;

    Display::HUD& GetHUD();

    Logging::ILogger* GetLogger() const;

    Scene::ISceneNode* GetScene() const;
    void SetScene(Scene::ISceneNode& scene);

    Display::Camera* GetCamera() const;
    void SetCamera(Display::Camera& volume);
    void SetCamera(Display::IViewingVolume& volume);


    Renderers::TextureLoader& GetTextureLoader();

    void AddDataDirectory(std::string dir);

    void EnableDebugging();
    
    void ShowFPS();

    // What about:
    // - HUD.
    // - Sound.
    // - Lighting.
    // - Physics.
    // - Input mappings.
    // - State machines.
    // Shall we let them wait? Not part of a simple setup?

    // Should we consider a more general Setup-System that consists of
    // different setup objects? What would such a system require and
    // why would it not suffer from the complexity of the current
    // structure?

private:
    std::string title;
    Core::IEngine* engine;
    Display::IEnvironment* env;
    Display::IFrame* frame;
    Display::IRenderCanvas* canvas;
    //Renderers::OpenGL::Renderer* renderer;
    Renderers::IRenderer* renderer;
    Devices::IMouse* mouse;
    Devices::IKeyboard* keyboard;
    Devices::IJoystick* joystick;
    Scene::ISceneNode* scene;
    Display::Camera* camera;
    Display::Frustum* frustum;
    Renderers::IRenderingView* renderingview;
    Renderers::OpenGL::LightRenderer* lightrenderer;
    Renderers::OpenGL::ShaderLoader* shaderloader;
    Renderers::TextureLoader* textureloader;
    Display::HUD* hud;
    Logging::ILogger* stdlog;
};

} // NS Utils
} // NS OpenEngine

#endif // _OE_SIMPLE_SETUP_H_
