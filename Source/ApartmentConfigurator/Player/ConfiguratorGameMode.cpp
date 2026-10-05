#include "ConfiguratorGameMode.h"
#include "ConfiguratorPlayerController.h"

#include "GameFramework/SpectatorPawn.h"

AConfiguratorGameMode::AConfiguratorGameMode()
{
	PlayerControllerClass = AConfiguratorPlayerController::StaticClass();

	// No gameplay character; a spectator pawn keeps the player valid while the camera
	// controller provides the actual view target.
	DefaultPawnClass = ASpectatorPawn::StaticClass();
}
