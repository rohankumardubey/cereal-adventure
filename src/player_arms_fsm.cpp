#include "../include/player_arms_fsm.h"

#include "../include/player.h"

c_adv::PlayerArmsFsm::PlayerArmsFsm() {
    m_player = nullptr;
    m_currentState = State::Idle;
}

c_adv::PlayerArmsFsm::~PlayerArmsFsm() {
    /* void */
}

void c_adv::PlayerArmsFsm::initialize(Player *player) {
    m_player = player;
}

void c_adv::PlayerArmsFsm::nextState(FsmResults &result) {
    ysVector velocity = m_player->RigidBody.GetVelocity();

    // FSM inputs
    State current = m_currentState;
    const bool onSurface = m_player->m_walkComponent.isOnSurface();
    const bool isHurt = m_player->isHurt();
    const bool isHanging = m_player->isHanging();
    const bool isGraspReady = m_player->isGraspReady();
    const bool isLaunching = m_player->isLaunching();
    const bool isActionComplete = m_player->isCurrentArmActionComplete();
    const float horizontalVelocity = ysMath::GetX(velocity);
    const bool isMoving = std::abs(horizontalVelocity) > 1.0f;
    const bool isAlive = m_player->isAlive();

    // FSM Output
    State next = current;
    State queued = State::Undefined;

    float nextFade = 0.0f;
    float nextClip = 0.0f;
    float queuedFade = 0.0f;
    float queuedClip = 0.0f;
    float speed = 1.0f;

    // FSM
    if (current == State::Idle) {
        if (onSurface) {
            if (!isAlive) {
                next = State::Dying;
                nextFade = 20.0f;
            }
            else if (isHurt) {
                next = State::ImpactDamage;
                nextFade = 20.0f;
            }
            else if (isMoving) {
                next = State::Running;
                nextFade = 20.0f;
            }
            else {
                queued = State::Idle;
            }
        }
        else if (isHanging) {
            next = State::Hanging;
            nextFade = 2.0f;
            nextClip = 30.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else if (isGraspReady) {
            next = State::Hanging;
            nextFade = 5.0f;
            nextClip = 10.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else {
            next = State::Idle;
            nextFade = 20.0f;

            queued = State::Idle;
            queuedFade = 0.0f;
        }
    }
    else if (current == State::Running) {
        if (onSurface) {
            if (!isAlive) {
                next = State::Dying;
                nextFade = 20.0f;
            }
            else if (isHurt) {
                next = State::ImpactDamage;
                nextFade = 20.0f;
            }
            else {
                if (!isMoving) {
                    next = State::Idle;
                    nextFade = 20.0f;
                }
                else {
                    next = State::Running;
                    queued = State::Running;
                    queuedFade = 0.0f;
                }
            }
        }
        else if (isHanging) {
            next = State::Hanging;
            nextFade = 2.0f;
            nextClip = 30.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else if (isGraspReady) {
            next = State::Hanging;
            nextFade = 10.0f;
            nextClip = 10.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else {
            next = State::Idle;
            nextFade = 20.0f;

            queued = State::Idle;
        }
    }
    else if (current == State::Hanging) {
        if (isHanging) {
            if (!isLaunching) {
                next = State::Hanging;
                nextFade = 2.0f;
                nextClip = 30.0f;

                queued = State::Hanging;
                queuedFade = 20.0f;
                queuedClip = 30.0f;
            }
            else {
                next = State::Launching;
                nextFade = 1.0f;
                speed = 1.5f;
            }
        }
        else if (isGraspReady) {
            next = State::Hanging;
            nextFade = 10.0f;
            nextClip = 10.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else if (onSurface) {
            if (isHurt) {
                next = State::ImpactDamage;
                nextFade = 20.0f;
            }
            else if (!isMoving) {
                next = State::Idle;
                nextFade = 20.0f;
            }
            else {
                next = State::Running;
                nextFade = 20.0f;
            }
        }
        else {
            next = State::Idle;
            nextFade = 40.0f;
        }
    }
    else if (current == State::ImpactDamage) {
        if (isHanging) {
            next = State::Hanging;
            nextFade = 2.0f;
            nextClip = 30.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else if (isGraspReady) {
            next = State::Hanging;
            nextFade = 10.0f;
            nextClip = 10.0f;

            queued = State::Hanging;
            queuedFade = 20.0f;
            queuedClip = 30.0f;
        }
        else if (onSurface) {
            if (!isHurt) {
                next = State::Idle;
                nextFade = 20.0f;
            }
        }
        else {
            next = State::Idle;
            nextFade = 40.0f;
        }
    }
    else if (current == State::Launching) {
        if (isActionComplete) {
            next = State::Idle;
            nextFade = 20.0f;
        }
    }

    // Output
    result.next = next;
    result.nextClip = nextClip;
    result.nextFade = nextFade;
    result.queued = queued;
    result.queuedClip = queuedClip;
    result.queuedFade = queuedFade;
    result.speed = speed;
}
