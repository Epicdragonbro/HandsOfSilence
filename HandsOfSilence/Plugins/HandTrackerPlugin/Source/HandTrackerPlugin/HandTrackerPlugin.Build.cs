//
// Copyright 2022 Adam Horvath - HAND.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

using UnrealBuildTool;
using System.IO;
using System;

public class HandTrackerPlugin : ModuleRules
{
	private void CopyToBinaries(string Filepath, string Dstpath)
	{
		string binariesDir = Dstpath;
		string filename = Path.GetFileName(Filepath);

		if (!Directory.Exists(binariesDir))
			Directory.CreateDirectory(binariesDir);

		if (!File.Exists(Path.Combine(binariesDir, filename)))
			File.Copy(Filepath, Path.Combine(binariesDir, filename), true);
	}

	private void CopyDirectory(string sourceDir, string destinationDir, bool recursive)
	{
		// Get information about the source directory
		var dir = new DirectoryInfo(sourceDir);

		// Check if the source directory exists
		if (!dir.Exists)
			throw new DirectoryNotFoundException("Source directory not found");

		// Cache directories before we start copying
		DirectoryInfo[] dirs = dir.GetDirectories();

		// Create the destination directory
		Directory.CreateDirectory(destinationDir);

		// Get the files in the source directory and copy to the destination directory
		foreach (FileInfo file in dir.GetFiles())
		{
			string targetFilePath = Path.Combine(destinationDir, file.Name);
			file.CopyTo(targetFilePath,true);
		}

		// If recursive and copying subdirectories, recursively call this method
		if (recursive)
		{
			foreach (DirectoryInfo subDir in dirs)
			{
				string newDestinationDir = Path.Combine(destinationDir, subDir.Name);
				CopyDirectory(subDir.FullName, newDestinationDir, true);
			}
		}
	}

	public HandTrackerPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
  				 "Core",
				 "CoreUObject",
				 "Engine",
				 "Projects" ,
				 "MediaAssets",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string SDKDIR = ModuleDirectory;

			


			//Copy .dll file to the project's binary folder
			string BinEditorPath = ModuleDirectory + "/../../../../Binaries/Win64/";
			string BinPath = SDKDIR + "/../../Binaries/ThirdParty/HandTrackerPluginLibrary/Win64/";

			PublicDelayLoadDLLs.AddRange(new string[] { BinEditorPath + "mediapipe_hand.dll",BinEditorPath + "opencv_world3410.dll"});

			CopyDirectory(BinPath + "mediapipe", BinEditorPath+"mediapipe",true);
			CopyToBinaries(BinPath + "mediapipe_hand.dll", BinEditorPath);
			CopyToBinaries(BinPath + "opencv_world3410.dll", BinEditorPath);

			RuntimeDependencies.Add(@"$(TargetOutputDir)/mediapipe",BinPath +"mediapipe/...");
			RuntimeDependencies.Add(@"$(TargetOutputDir)/mediapipe_hand.dll",BinPath +"mediapipe_hand.dll");
			RuntimeDependencies.Add(@"$(TargetOutputDir)/opencv_world3410.dll", BinPath +"opencv_world3410.dll");

			


		}
	}

}
