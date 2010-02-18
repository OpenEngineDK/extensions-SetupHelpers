// Simple setup of a graphics engine in OpenEngine.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Utils/SimpleSetup.h>

// Core stuff
#include <Core/Engine.h>
#include <Display/Camera.h>
#include <Display/Frustum.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Renderers/TextureLoader.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/SceneNode.h>

// Acceleration extension
#include <Renderers/AcceleratedRenderingView.h>
#include <Scene/ASDotVisitor.h>

// OpenGL extension
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Renderers/OpenGL/ShaderLoader.h>
#include <Renderers/OpenGL/LightRenderer.h>
#include <Resources/GLSLResource.h>

// SDL extension
#include <Display/SDLEnvironment.h>

// Resources extensions
#include <Resources/OBJResource.h>
//#include <Resources/ColladaResource.h>

// Logging
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>

// HUD
#include <Display/HUD.h>
#include <Utils/FPSSurface.h>

namespace OpenEngine {
namespace Utils {

using namespace Core;
using namespace Devices;
using namespace Display;
using namespace Logging;
using namespace Renderers::OpenGL;
using namespace Renderers;
using namespace Resources;
using namespace Scene;

class ExtRenderingView
    : public RenderingView
    , public AcceleratedRenderingView {
public:
    ExtRenderingView(Viewport& viewport)
        : IRenderingView(viewport)
        , RenderingView(viewport)
        , AcceleratedRenderingView(viewport) {}
};

class TextureLoadOnInit
    : public IListener<RenderingEventArg> {
    TextureLoader& tl;
public:
    TextureLoadOnInit(TextureLoader& tl) : tl(tl) { }
    void Handle(RenderingEventArg arg) {
        if (arg.renderer.GetSceneRoot() != NULL)
            tl.Load(*arg.renderer.GetSceneRoot());
    }
};

class QuitHandler : public IListener<KeyboardEventArg> {
    IEngine& engine;
public:
    QuitHandler(IEngine& engine) : engine(engine) {}
    void Handle(KeyboardEventArg arg) {
        if (arg.sym == KEY_ESCAPE) engine.Stop();
    }
};

/**
 * Create the simple setup helper.
 * This will create all of the engine components. After this you may
 * setup any additional project related components and then invoke
 * @code setup.GetEngine().Start() @endcode to start the engine. After
 * invoking the engine start the ordinary
 * initialize/process/deinitialize phases will proceed as usual.
 *
 * @param title Project title
 */
    SimpleSetup::SimpleSetup(std::string title, Display::Viewport* vp, Display::IEnvironment* env, Renderers::IRenderingView* rv, Core::IEngine* eng)
    : title(title)
    , engine(NULL)
    , env(NULL)
    , frame(NULL)
    , viewport(NULL)
    , renderer(NULL)
    , mouse(NULL)
    , keyboard(NULL)
    , joystick(NULL)
    , scene(NULL)
    , camera(NULL)
    , frustum(NULL)
    , renderingview(NULL)
    , textureloader(NULL)
    , hud(NULL)
{
    // create a logger to std out
    stdlog = new StreamLogger(&std::cout);
    Logger::AddLogger(stdlog);

    // setup the engine
    engine = (eng==NULL)?new Engine():eng;

    // setup display and devices
    this->env = env = (env == NULL) ? new SDLEnvironment(800,600) : env;
    frame    = &env->GetFrame();
    mouse    = env->GetMouse();
    keyboard = env->GetKeyboard();
    joystick = env->GetJoystick();
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);

    // setup a default viewport and camera
    viewport = (vp == NULL) ? new Viewport(*frame) : vp;
    camera  = new Camera(*(new PerspectiveViewingVolume()));
    frustum = new Frustum(*camera);
    viewport->SetViewingVolume(frustum);

    // add plug-ins
    ResourceManager<IModelResource>::AddPlugin(new OBJPlugin());
    ResourceManager<ITexture2D>::AddPlugin(new SDLImagePlugin());
    ResourceManager<IShaderResource>::AddPlugin(new GLSLPlugin());

    // populate the default scene
    scene = new SceneNode();
    scene->AddNode(new DirectionalLightNode());

    // setup the rendering system
    renderer = new Renderer(viewport);
    textureloader = new TextureLoader(*renderer);
    renderingview = (rv == NULL) ? new RenderingView(*viewport) : rv;
    lightrenderer = new LightRenderer(*viewport);

    engine->InitializeEvent().Attach(*renderer);
    engine->ProcessEvent().Attach(*renderer);
    engine->DeinitializeEvent().Attach(*renderer);

    renderer->PreProcessEvent().Attach(*lightrenderer);
    renderer->ProcessEvent().Attach(*renderingview);
    renderer->SetSceneRoot(scene);
    renderer->InitializeEvent().Attach(*(new TextureLoadOnInit(*textureloader)));
    renderer->PreProcessEvent().Attach(*textureloader);

    // bind default keys
    keyboard->KeyEvent().Attach(*(new QuitHandler(*engine)));
}

/**
 * Get the engine.
 * The engine can not be replaced.
 * @see IEngine
 */
IEngine& SimpleSetup::GetEngine() const {
    return *engine;
}

/**
 * Get the frame.
 * The default frame is will have the dimensions 800x600 at color depth 32.
 * The frame can not be replaced.
 * In order to change the frame use the set-methods defined in IFrame.
 * @see IFrame
 */
IFrame& SimpleSetup::GetFrame() const {
    return *frame;
}

/**
 * Get the renderer.
 * The renderer will automatically be supplied with a rendering
 * view that uses the full frame as its viewport.
 * Additionally it will always render the current scene supplied
 * by SetScene().
 * The renderer itself is not replaceable.
 */
IRenderer& SimpleSetup::GetRenderer() const {
    return *renderer;
}

/**
 * Get the mouse.
 * The mouse structure is not replaceable.
 */
IMouse& SimpleSetup::GetMouse() const {
    return *mouse;
}

/**
 * Get the keyboard.
 * The keyboard structure is not replaceable.
 */
IKeyboard& SimpleSetup::GetKeyboard() const {
    return *keyboard;
}

/**
 * Get the joystick.
 * The joystick structure is not replaceable.
 */
IJoystick& SimpleSetup::GetJoystick() const {
    return *joystick;
}

/**
 * Get the current scene.
 * The default scene consists of a SceneNode with a single
 * DirectionalLightNode beneath it.
 * If you wish to get rid of the light node simply delete the
 * scene and attache a new one that suits you purpose with
 * SetScene().
 * The current scene during renderer initialization will be
 * searched for textures to load.
 */
ISceneNode* SimpleSetup::GetScene() const {
    return scene;
}

/**
 * Set the current scene.
 * This will automatically update the renderer to use the new
 * scene.
 * Ownership of the scene remains with the caller but it is
 * assumed to be non-null as long as it is active.
 * If replacing a scene it is the callers responsibility to clean
 * up the old scene if needed by using GetScene().
 * When setting a new scene it will automatically be searched for
 * textures to load.
 * @param scene New active scene.
 */
void SimpleSetup::SetScene(ISceneNode& scene) {
    this->scene = &scene;
    renderer->SetSceneRoot(this->scene);
    textureloader->Load(scene);

    OpenGL::ShaderLoader* shaderLoader =
        new OpenGL::ShaderLoader(*textureloader, scene);
    engine->InitializeEvent().Attach(*shaderLoader);
}

/**
 * Get the current camera.
 * The default camera is placed in origin (0,0,0) following the
 * z-axis in the negative direction (0,0,-1).
 */
Camera* SimpleSetup::GetCamera() const {
    return camera;
}

/**
 * Set the current camera.
 * Ownership of the camera remains with the caller.
 */
void SimpleSetup::SetCamera(Camera& volume) {
    camera = &volume;
    delete frustum;
    frustum = new Frustum(*camera);
    viewport->SetViewingVolume(frustum);    
}
/**
 * Set a camera by viewing volume.
 * The non-camera class is used to create a *new* camera that wraps
 * the viewing volume. The new camera is created on the heap and the
 * caller is responsible for any needed clean-up of this structure.
 *
 * @param volume Volume to wrap in a *new* camera.
 */
void SimpleSetup::SetCamera(IViewingVolume& volume) {
    camera = new Camera(volume);
    viewport->SetViewingVolume(camera);
}

/**
 * Get a texture loader.
 * This texture loader has already been configured to the rendering
 * system and is ready for use.
 *
 * @return Texture loader.
 */
TextureLoader& SimpleSetup::GetTextureLoader() {
    return *textureloader;
}

/**
 * Add a data directory to the file search path.
 * This path will be searched when loading file resources.
 * By default we load all the resource plug-ins we can.
 *
 * @param dir Directory with file resources
 */
void SimpleSetup::AddDataDirectory(string dir) {
    DirectoryManager::AppendPath(dir);
}

HUD& SimpleSetup::GetHUD() {
    if (hud == NULL){
        // setup hud
        hud = new HUD();
        renderer->PostProcessEvent().Attach(*hud);
    }
    return *hud;
}

ILogger* SimpleSetup::GetLogger() const {
    return stdlog;
}

/**
 * Enable various run-time debugging features.
 * This includes
 * - visualization of the frustum,
 * - export the scene to a dot-graph file (scene.dot)
 * - add FPS to the HUD
 */
void SimpleSetup::EnableDebugging() {
    // Visualization of the frustum
    frustum->VisualizeClipping(true);
    scene->AddNode(frustum->GetFrustumNode());

    // @todo: this should be on initialize
    // Output a dot-graph of the scene
    ofstream dotfile("scene.dot", ofstream::out);
    if (!dotfile.good()) {
        logger.error << "Can not open 'scene.dot' for output"
                     << logger.end;
    } else {
        ASDotVisitor dot;
        dot.Write(*scene, &dotfile);
        logger.info << "Saved physics graph to 'scene.dot'"
                    << logger.end
                    << "To create a SVG image run: "
                    << "dot -Tsvg scene.dot > scene.svg"
                    << logger.end;
    }

    ShowFPS();
}
    
void SimpleSetup::ShowFPS() {
    // Setup fps counter
    FPSSurfacePtr fps = FPSSurface::Create();
    GetTextureLoader().Load(fps, TextureLoader::RELOAD_QUEUED);
    engine->ProcessEvent().Attach(*fps);
    HUD::Surface* fpshud = GetHUD().CreateSurface(fps);
    fpshud->SetPosition(HUD::Surface::LEFT, HUD::Surface::TOP);
        
        
}
    
} // NS Utils
} // NS OpenEngine
