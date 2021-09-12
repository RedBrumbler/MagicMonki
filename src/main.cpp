#include <thread>
#include "modloader/shared/modloader.hpp"
#include "GorillaLocomotion/Player.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils-functions.h"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "GlobalNamespace/OVRInput_Button.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/SphereCollider.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RaycastHit.hpp"
#include "UnityEngine/Physics.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Register.hpp"
#include "gorilla-utils/shared/GorillaUtils.hpp"
#include "gorilla-utils/shared/Callbacks/MatchMakingCallbacks.hpp"
#include "gorilla-utils/shared/Utils/Player.hpp"
#include "custom-types/shared/register.hpp"
#include "config.hpp"
#include "main.hpp"
#include "MagicMonkiWatchView.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

using namespace UnityEngine;
using namespace UnityEngine::XR;
using namespace GorillaLocomotion;
using namespace GlobalNamespace;

// Loads the config from disk using our modInfo, then returns it for use

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

bool inRoom = false;
bool TriggerLock = false;
int CtrlChoice = 0;
GameObject* pointer;

void UpdateValues() {
    CtrlChoice = config.ctrl;
}

void Teleport() {
    bool trigger = false;
    if (CtrlChoice == false) {
    trigger = OVRInput::Get(OVRInput::Button::PrimaryHandTrigger, OVRInput::Controller::RTouch);
    } else {
    trigger = OVRInput::Get(OVRInput::Button::PrimaryHandTrigger, OVRInput::Controller::LTouch);
    }
    if (inRoom && config.enabled) {
        if(trigger) {
            if (TriggerLock == false) {
                Transform* pointerTransform = pointer->get_transform();
                Vector3 pointerPos = pointerTransform->get_position();
                Player* playerInstance = Player::get_Instance();
                Transform* playerTransform = playerInstance->get_transform();
                Rigidbody* playerRigidBody = playerInstance->playerRigidBody;
                playerRigidBody->set_isKinematic(true);
                playerTransform->set_position(pointerPos+Vector3::get_down().get_normalized() * -1.0f);
                playerInstance->lastPosition = pointerPos+Vector3::get_down().get_normalized() * -1.0f;
                Array<Vector3>* velocityHistory = reinterpret_cast<Array<Vector3>*>(il2cpp_functions::array_new(classof(Vector3), playerInstance->velocityHistorySize));
                playerInstance->velocityHistory = velocityHistory;
                SphereCollider* headCollider = playerInstance->headCollider;
                Transform* headTransform = headCollider->get_transform();
                Vector3 headPos = headTransform->get_position();
                playerInstance->lastHeadPosition = headPos;
                Vector3 lastLeftHandPos = playerInstance->CurrentLeftHandPosition();
                playerInstance->lastLeftHandPosition = lastLeftHandPos;
                Vector3 lastRightHandPos = playerInstance->CurrentRightHandPosition();
                playerInstance->lastRightHandPosition = lastRightHandPos;
                playerRigidBody->set_isKinematic(false);
            }
            TriggerLock = true;
        } else TriggerLock = false;
    }
}

#include "GlobalNamespace/GorillaTagManager.hpp"

MAKE_HOOK_MATCH(GorillaTagManager_Update, &GlobalNamespace::GorillaTagManager::Update, void, GlobalNamespace::GorillaTagManager* self) {
    GorillaTagManager_Update(self);
    if (inRoom && config.enabled) {
    RaycastHit hitInfo;
    int layermask = 0b1 << 9;
    Player* playerInstance = Player::get_Instance();
    Rigidbody* playerPhysics = playerInstance->playerRigidBody;
    if(playerPhysics == nullptr) return;
    GameObject* playerGameObject = playerPhysics->get_gameObject();
    if(playerGameObject == nullptr) return;
    auto* player = playerGameObject->GetComponent<GorillaLocomotion::Player*>();
    //UnityEngine::Transform* transform = playerGameObject->get_transform();
    Transform* rightHandT = player->rightHandTransform;
    if (CtrlChoice == true) {
    Transform* rightHandT = player->leftHandTransform;
    }
    Physics::Raycast(rightHandT->get_position()+ Vector3::get_down().get_normalized() * 0.1f, rightHandT->get_up()*-0.1f, hitInfo, 300.0f, layermask);
    float rayDistance = hitInfo.get_distance();
    if (pointer == nullptr) {
            pointer = GameObject::CreatePrimitive(PrimitiveType::Sphere);
            pointer->get_transform()->set_localScale(Vector3(0.2,0.2,0.2));
            GameObject::Destroy(pointer->GetComponent<Collider*>());
    }
    if (rayDistance > 0.1f && rayDistance < 300.0f) {
        pointer->get_transform()->set_position(hitInfo.get_point());
    }
    Teleport();
    } else {
        if (pointer == nullptr) {
        } else {
            pointer = nullptr;
        }
    }
}
MAKE_HOOK_MATCH(Player_Awake, &GorillaLocomotion::Player::Awake, void, GorillaLocomotion::Player* self) {
    Player_Awake(self);
    UpdateValues();
    GorillaUtils::MatchMakingCallbacks::onJoinedRoomEvent() += {[&]() {
        Il2CppObject* currentRoom = CRASH_UNLESS(il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "get_CurrentRoom"));

        if (currentRoom)
        {
            inRoom = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
        } else inRoom = true;
    }
    };
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    GorillaUI::Init();
    il2cpp_functions::Init();
    INSTALL_HOOK(getLogger(), Player_Awake);
    INSTALL_HOOK(getLogger(), GorillaTagManager_Update);
    GorillaUI::Register::RegisterWatchView<MagicMonki::MagicMonkiWatchView*>("<b><i><color=#c53ab8>Teleportin Monki</color></i></b>", VERSION);
    getLogger().info("Installing hooks...");
    // Install our hooks (none defined yet)
    LoadConfig();
    getLogger().info("Installed all hooks!");
}