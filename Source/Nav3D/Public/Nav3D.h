#pragma once

#include "Console/FUConsole.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNav3D, Log, All);


#if FU_WITH_CONSOLE
	#define NAV3D_WITH_DEBUG 1
#else 
	#define NAV3D_WITH_DEBUG 0
#endif


class FNav3DModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
