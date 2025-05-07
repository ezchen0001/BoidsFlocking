#include "FlockingManager.h"
#include "Agent.h"

#include <vector>

#define AGENT_COUNT 20    
#define AGENT_TARGET_SPEED 3.f
#define NEIGHBOR_RADIUS 3000.f
#define PERSONAL_RADIUS 300.f

void UFlockingManager::Init(UWorld* world, UStaticMeshComponent* mesh) {
    UE_LOG(LogTemp, Warning, TEXT("MANAGER INIT"));

    World = world;

    float incr = (PI * 2.f) / AGENT_COUNT;
    for (int i = 0; i < AGENT_COUNT; i++) {
        if (World != nullptr) {
            FRotator rotation = FRotator();

            FVector location = FVector();
            location.X = FMath::Sin(incr * i) * 150.f;
            location.Y = FMath::Cos(incr * i) * 150.f;

            AAgent* agent = World->SpawnActor<AAgent>(location, rotation);
            agent->Init(mesh, i);
            Agents.Add(agent);
        }
    }

    initialized = true;
}

void UFlockingManager::Flock() {
    const float flockInfluence = 0.0001; // How much flocking rules influence velocity

    auto camera = World->GetFirstPlayerController()->PlayerCameraManager;

    for (int i = 0; i < AGENT_COUNT; i++) {
        AAgent* agent = Agents[i];
        auto pos = agent->GetActorLocation();
        std::vector<AAgent*> flockNeighbors;

        FVector force(0.f);

        // Move towards camera
        auto camLoc = camera->GetCameraLocation();
        force += camLoc - pos;
        force.Normalize();
        force *= 0.01;

        // Find neighbors
        for (int j = 0; j < AGENT_COUNT; j++) {
            AAgent* otherAgent = Agents[j];
            if (agent == otherAgent){
                continue;
            }
            auto otherPos = otherAgent->GetActorLocation();
            if ((otherPos - pos).Length() < NEIGHBOR_RADIUS) {
                flockNeighbors.push_back(otherAgent);
            }
        }

        // Find flocking forces
        if (!flockNeighbors.empty()) {
            FVector centerAvg(0.f); // Rule 1, track center of flock
            FVector keepAway(0.f); // Rule 2, maintain distance from flock
            FVector velocityAvg(0.f); // Rule 3, track direction of flock
            for (auto* neighbor : flockNeighbors) {
                auto otherPos = neighbor->GetActorLocation();
                centerAvg += otherPos; // Sum positions for center average
                if ((otherPos - pos).Length() < PERSONAL_RADIUS) {
                    keepAway += pos - otherPos; // Sum offsets for keep away
                }
                velocityAvg += neighbor->Velocity; // sum velocities for average direction
            }

            centerAvg /= flockNeighbors.size();
            velocityAvg /= flockNeighbors.size();

            force += (centerAvg - pos) * flockInfluence;
            force += keepAway * flockInfluence * 2;
            force += velocityAvg * flockInfluence;
        }

        // Apply force to velocity and normalize
        
        agent->Velocity += force;
        if (!agent->Velocity.IsZero()) {
            agent->Velocity.Normalize();
            agent->Velocity *= AGENT_TARGET_SPEED;
        }

        
    }
}