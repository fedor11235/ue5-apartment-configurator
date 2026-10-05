#include "ConfiguratorGameMode.h"
#include "ConfiguratorPlayerController.h"

AConfiguratorGameMode::AConfiguratorGameMode()
{
	PlayerControllerClass = AConfiguratorPlayerController::StaticClass();

	// No gameplay character: the camera controller provides the view target and all input
	// is cursor/UI driven, so a pawn would only intercept mouse look. Spawn none.
	DefaultPawnClass = nullptr;
}
