//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#include "Core.h"
#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/MinWindows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif 

#include "IHandTrackerPlugin.h"
#include "MediaPipeConnector.h"

class FHandTrackerPlugin : public IHandTrackerPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};


IMPLEMENT_MODULE(FHandTrackerPlugin, HandTrackerPlugin)

void FHandTrackerPlugin::StartupModule()
{
	TSharedPtr<FMediaPipeHandConnector> MediaPipeStartup(new FMediaPipeHandConnector);
	if (MediaPipeStartup->StartupConnector())
	{
		MediaPipeConnector = MediaPipeStartup;
	}

}

void FHandTrackerPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (MediaPipeConnector.IsValid())
	{
		MediaPipeConnector->ShutdownConnector();
		MediaPipeConnector = nullptr;
	}


	
}

#undef LOCTEXT_NAMESPACE
	

