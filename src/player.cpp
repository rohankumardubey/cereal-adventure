#include "../include/player.h"

#include "../include/world.h"

#include <sstream>

ysAnimationAction
*c_adv::Player::AnimArmsWalk = nullptr,
*c_adv::Player::AnimLegsWalk = nullptr,
*c_adv::Player::AnimLegsIdle = nullptr,
*c_adv::Player::AnimArmsIdle = nullptr;

dbasic::SceneObjectAsset *c_adv::Player::CharacterRoot = nullptr;

c_adv::Player::Player() {
    m_armsChannel = nullptr;
    m_legsChannel = nullptr;
    m_renderSkeleton = nullptr;
}

c_adv::Player::~Player() {
    /* void */
}

void c_adv::Player::initialize() {
    GameObject::initialize();

    RigidBody.SetHint(dphysics::RigidBody::RigidBodyHint::Dynamic);
    RigidBody.SetInverseMass(1.0f);
    RigidBody.SetAlwaysAwake(true);
    RigidBody.SetRequestsInformation(true);

    dphysics::CollisionObject *bounds;
    RigidBody.CollisionGeometry.NewBoxObject(&bounds);
    bounds->SetMode(dphysics::CollisionObject::Mode::Fine);
    bounds->GetAsBox()->HalfHeight = 1.0f;
    bounds->GetAsBox()->HalfWidth = 0.75f;
    bounds->GetAsBox()->Orientation = ysMath::Constants::QuatIdentity;
    bounds->GetAsBox()->Position = ysMath::Constants::Zero;

    m_renderSkeleton = m_world->getAssetManager().BuildRenderSkeleton(
        &RigidBody.Transform, CharacterRoot);

    m_renderSkeleton->BindAction(AnimArmsWalk, &m_animArmsWalk);
    m_renderSkeleton->BindAction(AnimLegsWalk, &m_animLegsWalk);
    m_renderSkeleton->BindAction(AnimLegsIdle, &m_animLegsIdle);
    m_renderSkeleton->BindAction(AnimArmsIdle, &m_animArmsIdle);

    m_legsChannel = m_renderSkeleton->AnimationMixer.NewChannel();
    m_armsChannel = m_renderSkeleton->AnimationMixer.NewChannel();
}

void c_adv::Player::process() {
    GameObject::process();

    updateMotion();
    updateAnimation();
}

void c_adv::Player::render() {
    int color[] = { 0xf1, 0xc4, 0x0f };
    m_world->getEngine().SetObjectTransform(RigidBody.Transform.GetWorldTransform());
    //m_world->getEngine().DrawBox(color, 5.0f, 5.0f, (int)Layer::Player);

    m_world->getEngine().SetMultiplyColor(ysVector4(1.0f, 1.0f, 1.0f, 1.0f));
    m_world->getEngine().DrawRenderSkeleton(m_renderSkeleton, 1.0f, (int)Layer::Player);

    dbasic::Console *console = m_world->getEngine().GetConsole();
    console->Clear();
    console->MoveToLocation(dbasic::GuiPoint(1, 2));
    console->SetFontForeColor(0, 0, 0, 1.0f);
    console->SetFontBackColor(0, 0, 0, 0.0f);

    std::stringstream msg;
    ysVector position = RigidBody.Transform.GetWorldPosition();
    msg << "Pos " << ysMath::GetX(position) << "/" << ysMath::GetY(position) << "\n";
    msg << "FPS " << m_world->getEngine().GetAverageFramerate() << "\n";
    msg << "AO/DO/VI: " << m_realm->getAliveObjectCount() << "/" << m_realm->getDeadObjectCount() << "/" << m_realm->getVisibleObjectCount() << "\n";
    console->DrawGeneralText(msg.str().c_str());
}

void c_adv::Player::updateMotion() {
    dbasic::DeltaEngine &engine = m_world->getEngine();

    if (engine.IsKeyDown(ysKeyboard::KEY_D)) {
        RigidBody.SetVelocity(ysMath::LoadVector(3.0f, 0.0f, 0.0f));
    }
    else RigidBody.SetVelocity(ysMath::LoadVector(0.0f, 0.0f, 0.0f));
}

void c_adv::Player::updateAnimation() {
    ysVector velocity = RigidBody.GetVelocity();
    float mag = ysMath::GetScalar(ysMath::MagnitudeSquared3(velocity));

    if (mag > 0.01f) {
        ysAnimationChannel::ActionSettings smooth;
        smooth.FadeIn = 20.0f;
        smooth.Speed = 1.0f;

        ysAnimationChannel::ActionSettings immediate;
        immediate.FadeIn = 0.0f;
        immediate.Speed = 1.0f;

        if (m_legsChannel->GetCurrentAction() != &m_animLegsWalk) {
            m_legsChannel->AddSegment(&m_animLegsWalk, smooth);
            m_legsChannel->ClearQueue();
        }
        else if (!m_legsChannel->HasQueuedSegments()) {
            m_legsChannel->QueueSegment(&m_animLegsWalk, immediate);
        }

        if (m_armsChannel->GetCurrentAction() != &m_animArmsWalk) {
            m_armsChannel->AddSegment(&m_animArmsWalk, smooth);
            m_armsChannel->ClearQueue();
        }
        else if (!m_armsChannel->HasQueuedSegments()) {
            m_armsChannel->QueueSegment(&m_animArmsWalk, immediate);
        }
    }
    else {
        ysAnimationChannel::ActionSettings smooth;
        smooth.FadeIn = 20.0f;
        smooth.Speed = 1.0f;

        ysAnimationChannel::ActionSettings immediate;
        immediate.FadeIn = 0.0f;
        immediate.Speed = 1.0f;

        if (m_legsChannel->GetCurrentAction() != &m_animLegsIdle) {
            m_legsChannel->AddSegment(&m_animLegsIdle, smooth);
            m_legsChannel->ClearQueue();
        }
        else if (!m_legsChannel->HasQueuedSegments()) {
            m_legsChannel->QueueSegment(&m_animLegsIdle, immediate);
        }

        if (m_armsChannel->GetCurrentAction() != &m_animArmsIdle) {
            m_armsChannel->AddSegment(&m_animArmsIdle, smooth);
            m_armsChannel->ClearQueue();
        }
        else if (!m_armsChannel->HasQueuedSegments()) {
            m_armsChannel->QueueSegment(&m_animArmsIdle, immediate);
        }
    }

    m_renderSkeleton->UpdateAnimation(m_world->getEngine().GetFrameLength() * 60.0f);
}

void c_adv::Player::configureAssets(dbasic::AssetManager *am) {
    AnimLegsWalk = am->GetAction("LegsRun");
    AnimArmsWalk = am->GetAction("ArmsRun");
    AnimLegsIdle = am->GetAction("LegsIdle");
    AnimArmsIdle = am->GetAction("ArmsIdle");

    AnimLegsWalk->SetLength(40.0f);
    AnimArmsWalk->SetLength(40.0f);
    AnimLegsIdle->SetLength(100.0f);
    AnimArmsIdle->SetLength(100.0f);

    CharacterRoot = am->GetSceneObject("CerealArmature");
}