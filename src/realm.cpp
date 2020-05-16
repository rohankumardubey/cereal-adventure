#include "../include/realm.h"

#include "../include/game_object.h"
#include "../include/world.h"

c_adv::Realm::Realm() {
    m_exitPortal = nullptr;
    m_world = nullptr;
    m_indoor = false;

    m_visibleObjectCount = 0;
}

c_adv::Realm::~Realm() {
    /* void */
}

void c_adv::Realm::registerGameObject(GameObject *object) {
    object->setRealm(this);
    object->setRealmRecordIndex((int)m_gameObjects.size());
    m_gameObjects.push_back(object);

    PhysicsSystem.RegisterRigidBody(&object->RigidBody);
}

void c_adv::Realm::unregisterGameObject(GameObject *object) {
    int index = object->getRealmRecordIndex();

    object->setRealmRecordIndex(-1);
    PhysicsSystem.RemoveRigidBody(&object->RigidBody);

    m_gameObjects[index] = m_gameObjects.back();
    m_gameObjects[index]->setRealmRecordIndex(index);
    m_gameObjects.pop_back();
}

void c_adv::Realm::process() {
    cleanObjectList();
    spawnObjects();
    respawnObjects();

    for (GameObject *g : m_gameObjects) {
        if (g->getDeletionFlag()) continue;

        g->createVisualBounds();
        g->process();
    }

    PhysicsSystem.Update(getEngine().GetFrameLength());
}
 
void c_adv::Realm::render() {
    m_world->getEngine().SetClearColor(0x0b, 0x03, 0x2d);
    m_world->getEngine().SetClearColor(0, 0, 0);

    m_world->getEngine().SetAmbientLight(ysVector4(0.6f, 0.5f, 0.5f));
                   
    dbasic::Light sun;    
    sun.Position = ysVector4(10.0f, 10.0f, 10.0f); 
    sun.Color = ysVector4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 0.0f);
    sun.Color.Scale(0.3f);
    sun.FalloffEnabled = 0;
    m_world->getEngine().AddLight(sun);

    dbasic::Light backLight;  
    backLight.Position = ysVector4(7.0f, 1.0f, 2.0f);    
    backLight.Color = ysVector4(43 / 255.0f, 209 / 255.0f, 252 / 255.0f, 0.0f);
    backLight.Color.Scale(4.0f);
    backLight.FalloffEnabled = 1;

    AABB cameraExtents = m_world->getCameraExtents();

    int visibleObjects = 0;

    for (GameObject *g : m_gameObjects) {
        if (g->getDeletionFlag()) continue;
        AABB extents = g->getVisualBounds();

        if (extents.intersects2d(cameraExtents) || true) {
            g->render();
            ++visibleObjects;
        }
    }

    m_visibleObjectCount = visibleObjects;
}

void c_adv::Realm::spawnObjects() {
    while (!m_spawnQueue.empty()) {
        GameObject *u = m_spawnQueue.front(); m_spawnQueue.pop();
        u->initialize();
        registerGameObject(u);
    }
}

void c_adv::Realm::respawnObjects() {
    while (!m_respawnQueue.empty()) {
        GameObject *u = m_respawnQueue.front(); m_respawnQueue.pop();
        registerGameObject(u);
    }
}

dbasic::DeltaEngine &c_adv::Realm::getEngine() {
    return m_world->getEngine();
}

void c_adv::Realm::updateRealms() {
    int N = (int)m_gameObjects.size();
    for (int i = 0; i < N; ++i) {
        if (m_gameObjects[i]->isChangingRealm()) {
            GameObject *object = m_gameObjects[i];
            Realm *newRealm = object->getNewRealm();
            object->resetRealmChange();

            unregisterGameObject(object);

            if (newRealm != nullptr) {
                newRealm->registerGameObject(object);
            }

            GameObject *lastPortal = object->getLastPortal();
            object->setLastPortal(nullptr);

            if (lastPortal != nullptr) {
                if (newRealm == lastPortal->getTargetRealm()) {
                    lastPortal->onEnter(object);
                }
                else if (newRealm == lastPortal->getRealm()) {
                    lastPortal->onExit(object);
                }
            }

            --i;
            N = (int)m_gameObjects.size();
        }
    }
}

void c_adv::Realm::unload(GameObject *object) {
    m_unloadQueue.push(object);
}

void c_adv::Realm::respawn(GameObject *object) {
    m_respawnQueue.push(object);
}

void c_adv::Realm::addToSpawnQueue(GameObject *object) {
    m_spawnQueue.push(object);
}

void c_adv::Realm::cleanObjectList() {
    int N = (int)m_gameObjects.size();
    for (int i = 0; i < N; ++i) {
        if (m_gameObjects[i]->getDeletionFlag()) {
            m_deadObjects.push_back(m_gameObjects[i]);
            unregisterGameObject(m_gameObjects[i]);

            --i; --N;
        }
    }

    int N_dead = (int)m_deadObjects.size();
    for (int i = 0; i < N_dead; ++i) {
        if (m_deadObjects[i]->getReferenceCount() == 0) {
            destroyObject(m_deadObjects[i]);

            m_deadObjects[i] = m_deadObjects.back();
            m_deadObjects.pop_back();

            --i; --N_dead;
        }
    }

    int N_unloaded = (int)m_unloadQueue.size();
    for (int i = 0; i < N_unloaded; ++i) {
        GameObject *u = m_unloadQueue.front(); m_unloadQueue.pop();
        unregisterGameObject(u);
    }
}

void c_adv::Realm::destroyObject(GameObject *object) {
    _aligned_free((void *)object);
}
