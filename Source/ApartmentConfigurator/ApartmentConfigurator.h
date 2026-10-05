#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Primary game module. Declares a dedicated log category used across the configurator
// so that data-loading / camera / interaction messages are easy to filter in the output log.
DECLARE_LOG_CATEGORY_EXTERN(LogConfigurator, Log, All);
