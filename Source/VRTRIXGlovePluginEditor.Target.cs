// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class VRTRIXGlovePluginEditorTarget : TargetRules
{
	public VRTRIXGlovePluginEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

        // UE 5.8 refuses an editor target whose settings differ from the installed engine's.
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

        ExtraModuleNames.AddRange( new string[] { "VRTRIXGlovePlugin" } );
	}
}
