// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class VRTRIXGlovePluginEditorTarget : TargetRules
{
	public VRTRIXGlovePluginEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

        // The installed engine's own settings (UE 5.7: V6, include order 5.7), as the editor target
        // shares build products with UnrealEditor.
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

        ExtraModuleNames.AddRange( new string[] { "VRTRIXGlovePlugin" } );
	}
}
