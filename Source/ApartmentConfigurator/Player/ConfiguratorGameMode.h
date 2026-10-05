#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ConfiguratorGameMode.generated.h"

/**
 * Minimal game mode. The experience is UI + camera driven, so there is no gameplay pawn —
 * viewing is handled by the ConfiguratorPlayerController taking over the level camera.
 */
UCLASS()
class APARTMENTCONFIGURATOR_API AConfiguratorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AConfiguratorGameMode();
};
