//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#pragma once

#include "Modules/ModuleManager.h"


/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IHandTrackerPlugin : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IHandTrackerPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< IHandTrackerPlugin >( "HandTrackerPlugin" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "HandTrackerPlugin" );
	}

	FORCEINLINE TSharedPtr<class FMediaPipeHandConnector> GetMediaPipeConnector() const
	{
		return MediaPipeConnector;
	}

	/**
	* Simple helper function to get the device currently active.
	* @return	Pointer to the MediaPipeConnector, or nullptr if Device is not available.
	*/
	static FMediaPipeHandConnector* GetMediaPipeConnectorSafe()
	{
#if WITH_EDITOR
		FMediaPipeHandConnector* MediaPipeConnector = IHandTrackerPlugin::IsAvailable() ? IHandTrackerPlugin::Get().GetMediaPipeConnector().Get() : nullptr;
#else
		FMediaPipeHandConnector* MediaPipeConnector = IHandTrackerPlugin::Get().GetMediaPipeConnector().Get();
#endif
		return MediaPipeConnector;
	}


protected:
	/**
	* Reference to the actual MediaPipeConnector, grabbed through the GetMediaPipeConnector() interface, and created and destroyed in Startup/ShutdownModule
	*/
	TSharedPtr<class FMediaPipeHandConnector> MediaPipeConnector;
};