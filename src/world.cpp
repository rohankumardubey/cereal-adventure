#include "../include/world.h"

#include "../include/asset_loader.h"
#include "../include/realm.h"
#include "../include/player.h"
#include "../include/test_obstacle.h"

#include "../include/game_objects.h"

const float c_adv::World::DefaultCameraDistance = 10.0f;
const std::string c_adv::World::PhysicsTimer = "Physics";

c_adv::World::World() {
    m_focus = nullptr;
    m_mainRealm = nullptr;
    m_cameraDistance = DefaultCameraDistance;
    m_respawnPosition = ysMath::Constants::Zero;
}

c_adv::World::~World() {
    /* void */
}

void c_adv::World::initialize(void *instance, ysContextObject::DeviceAPI api) {
    dbasic::Path modulePath = dbasic::GetModulePath();
    dbasic::Path confPath = modulePath.Append("delta.conf");

    std::string enginePath = "../../dependencies/delta/engines/basic";
    std::string assetPath = "../../assets";
    std::string loggingPath = "../../workspace";
    std::string shaderPath;
    if (confPath.Exists()) {
        std::fstream confFile(confPath.ToString(), std::ios::in);

        std::getline(confFile, enginePath);
        std::getline(confFile, assetPath);
        enginePath = modulePath.Append(enginePath).ToString();
        assetPath = modulePath.Append(assetPath).ToString();
        loggingPath = modulePath.ToString();

        confFile.close();
    }

    shaderPath = enginePath + "/shaders/";
    m_assetPath = dbasic::Path(assetPath);

    m_engine.GetConsole()->SetDefaultFontDirectory(enginePath + "/fonts/");

    dbasic::DeltaEngine::GameEngineSettings settings;
    settings.API = api;
    settings.DepthBuffer = true;
    settings.FrameLogging = true;
    settings.Instance = instance;
    settings.LoggingDirectory = loggingPath.c_str();
    settings.ShaderDirectory = shaderPath.c_str();
    settings.WindowTitle = "Cereal Adventure // Development Build // " __DATE__;

    // Create timers
    m_engine.GetBreakdownTimer().CreateChannel(PhysicsTimer);

    m_engine.CreateGameWindow(settings);

    m_assetManager.SetEngine(&m_engine);

    AssetLoader::loadAllAssets(dbasic::Path(assetPath), &m_assetManager);

    // Camera settings
    m_engine.SetCameraMode(dbasic::DeltaEngine::CameraMode::Target);

    m_smoothCamera.setPosition(ysMath::Constants::Zero);
    m_smoothCamera.setStiffnessTensor(ysMath::LoadVector(200.0f, 50.0f, 0.0f));
    m_smoothCamera.setDampingTensor(ysMath::LoadVector(0.3f, 0.5f, 0.0f));

    m_smoothTarget.setPosition(ysMath::Constants::Zero);
    m_smoothTarget.setStiffnessTensor(ysMath::LoadVector(250.0f, 100.0f, 0.0f));
    m_smoothTarget.setDampingTensor(ysMath::LoadVector(0.3f, 0.5f, 0.0f));

    m_engine.SetCursorHidden(false);
    m_engine.SetCursorPositionLock(false);
}

void c_adv::World::initialSpawn() {
    m_mainRealm = newRealm<Realm>();
    m_mainRealm->setIndoor(false);

    ysTransform root;
    dbasic::RenderSkeleton *level1 = m_assetManager.BuildRenderSkeleton(&root, m_assetManager.GetSceneObject("Level 1"));
    generateLevel(level1);

    m_focus = m_mainRealm->spawn<Player>();
    m_focus->RigidBody.Transform.SetPosition(m_respawnPosition);
    m_focus->incrementReferenceCount();
}

void c_adv::World::run() {
    initialSpawn();

    while (m_engine.IsOpen()) {
        frameTick();
    }
}

void c_adv::World::frameTick() {
    m_engine.StartFrame();

    process();
    render();

    m_engine.EndFrame();
}

c_adv::AABB c_adv::World::getCameraExtents() const {
    float cameraX, cameraY;
    m_engine.GetCameraPosition(&cameraX, &cameraY);

    float altitude = m_engine.GetCameraAltitude();

    float v, h;
    float fov = m_engine.GetCameraFov();
    float aspect = m_engine.GetCameraAspect();

    v = std::tan(fov / 2) * altitude;
    h = v * aspect;

    return { ysMath::LoadVector(-h + cameraX, -v + cameraY, 0.0f, 0.0f), ysMath::LoadVector(h + cameraX, v + cameraY) };
}

void c_adv::World::render() {
    float offset_x = 0.0f, offset_y = 0.0f;
    if (m_engine.IsKeyDown(ysKeyboard::KEY_DOWN)) {
        offset_y = -5.0f;
    }
    else if (m_engine.IsKeyDown(ysKeyboard::KEY_UP)) {
        offset_y = 5.0f;
    }

    ysVector focusPosition = m_focus->RigidBody.Transform.GetWorldPosition();
    m_smoothCamera.setTarget(focusPosition);
    m_smoothTarget.setTarget(ysMath::Add(focusPosition, ysMath::LoadVector(offset_x, offset_y, 0.0f)));

    ysVector cameraTarget = m_smoothCamera.getPosition();

    m_engine.SetCameraPosition(ysMath::GetX(cameraTarget), ysMath::GetY(cameraTarget));
    m_engine.SetCameraAltitude(m_cameraDistance);
    m_engine.SetCameraTarget(m_smoothTarget.getPosition());
    m_engine.SetCameraUp(ysMath::Constants::YAxis);

    m_mainRealm->render();
}

void c_adv::World::process() {
    // Limit min framerate to 30 fps
    const float dt = min(1 / 30.0f, getEngine().GetFrameLength());

    m_mainRealm->process(dt);

    m_smoothCamera.update(dt);
    m_smoothTarget.update(dt);

    if (m_engine.IsKeyDown(ysKeyboard::KEY_SUBTRACT)) {
        m_cameraDistance += 0.5f;
    }
    else if (m_engine.IsKeyDown(ysKeyboard::KEY_ADD)) {
        m_cameraDistance -= 0.5f;
        if (m_cameraDistance < 1.0f) m_cameraDistance = 1.0f;
    }
    else if (m_engine.IsKeyDown(ysKeyboard::KEY_BACK)) {
        m_cameraDistance = DefaultCameraDistance;
    }

    if (m_focus->isDead()) {
        m_focus->decrementReferenceCount();
        m_focus = m_mainRealm->spawn<Player>();
        m_focus->RigidBody.Transform.SetPosition(m_respawnPosition);
        m_focus->incrementReferenceCount();
    }
}

void c_adv::World::generateLevel(dbasic::RenderSkeleton *hierarchy) {
    for (int i = 0; i < hierarchy->GetNodeCount(); ++i) {
        dbasic::RenderNode *node = hierarchy->GetNode(i);
        dbasic::SceneObjectAsset *sceneAsset = node->GetSceneAsset();
        if (strcmp(sceneAsset->GetName(), "Ledge") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Ledge *newLedge = m_mainRealm->spawn<Ledge>();
            newLedge->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Counter_1") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Counter *newCounter = m_mainRealm->spawn<Counter>();
            newCounter->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Toaster") == 0) { 
            ysVector position = node->Transform.GetWorldPosition();
            Toaster *newToaster = m_mainRealm->spawn<Toaster>();
            newToaster->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Shelves") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Shelves *newShelves = m_mainRealm->spawn<Shelves>();
            newShelves->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Fridge") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Fridge *newFridge = m_mainRealm->spawn<Fridge>();
            newFridge->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Stool_1") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Stool_1 *newStool = m_mainRealm->spawn<Stool_1>();
            newStool->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Microwave") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Microwave *newMicrowave = m_mainRealm->spawn<Microwave>();
            newMicrowave->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Oven") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Oven *newOven = m_mainRealm->spawn<Oven>();
            newOven->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "SingleShelf") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            SingleShelf *newOven = m_mainRealm->spawn<SingleShelf>();
            newOven->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Vase") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Vase *newVase = m_mainRealm->spawn<Vase>();
            newVase->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Cabinet") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Cabinet *newCabinet = m_mainRealm->spawn<Cabinet>();
            newCabinet->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Sink") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Sink *newSink = m_mainRealm->spawn<Sink>();
            newSink->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "PlayerStart") == 0) {
            m_respawnPosition = node->Transform.GetWorldPosition();
        }
        else if (strcmp(sceneAsset->GetName(), "LightSource::Ceiling") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            CeilingLightSource *newLight = m_mainRealm->spawn<CeilingLightSource>();
            newLight->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "StoveHood") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            StoveHood *stoveHood = m_mainRealm->spawn<StoveHood>();
            stoveHood->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "FruitBowl") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            FruitBowl *fruitBowl = m_mainRealm->spawn<FruitBowl>();
            fruitBowl->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Fan") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Fan *fan = m_mainRealm->spawn<Fan>();
            fan->RigidBody.Transform.SetPosition(position);
        }
        else if (strcmp(sceneAsset->GetName(), "Table") == 0) {
            ysVector position = node->Transform.GetWorldPosition();
            Table *table = m_mainRealm->spawn<Table>();
            table->RigidBody.Transform.SetPosition(position);
        }
        else if (sceneAsset->GetType() == ysObjectData::ObjectType::Instance) {
            ysVector position = node->Transform.GetWorldPosition();
            ysQuaternion orientation = node->Transform.GetWorldOrientation();

            StaticArt *newStaticArt = m_mainRealm->spawn<StaticArt>();
            newStaticArt->setAsset(node->GetModelAsset());
            newStaticArt->RigidBody.Transform.SetPosition(position);
            newStaticArt->RigidBody.Transform.SetOrientation(orientation);
        }
    }
}

void c_adv::World::updateRealms() {
    for (Realm *realm : m_realms) {
        realm->updateRealms();
    }
}
