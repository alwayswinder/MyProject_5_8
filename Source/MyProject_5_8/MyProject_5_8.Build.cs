// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyProject_5_8 : ModuleRules
{
	public MyProject_5_8(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
