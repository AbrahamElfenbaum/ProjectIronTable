// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class ProjectIronTable : ModuleRules
{
	public ProjectIronTable(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Dice"),
			Path.Combine(ModuleDirectory, "UI"),
			Path.Combine(ModuleDirectory, "Chat"),
            Path.Combine(ModuleDirectory, "Utility"),
            Path.Combine(ModuleDirectory, "Pawns"),
            Path.Combine(ModuleDirectory, "PlayerControllers"),
            Path.Combine(ModuleDirectory, "PlayerList"),
            Path.Combine(ModuleDirectory, "SaveLoad"),
            Path.Combine(ModuleDirectory, "Components"),
            Path.Combine(ModuleDirectory, "Settings"),
            Path.Combine(ModuleDirectory, "CampaignManager"),
            Path.Combine(ModuleDirectory, "CampaignBrowser"),
            Path.Combine(ModuleDirectory, "AssetLibrary"),
            Path.Combine(ModuleDirectory, "GameStates"),
            Path.Combine(ModuleDirectory, "PlayerStates"),
            Path.Combine(ModuleDirectory, "GameModes"),
            Path.Combine(ModuleDirectory, "GameInstances"),
            Path.Combine(ModuleDirectory, "SessionNotes")

        });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysicsCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
