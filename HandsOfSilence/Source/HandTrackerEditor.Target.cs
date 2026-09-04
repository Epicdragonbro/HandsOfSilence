// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HandTrackerEditorTarget : TargetRules
{
	public HandTrackerEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
	        ExtraModuleNames.AddRange( new string[] { "HandTracker" } );
        	bOverrideBuildEnvironment = true;
	        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
	        DefaultBuildSettings = BuildSettingsVersion.V6;
	 }
}
