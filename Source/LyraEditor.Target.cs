// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraEditorTarget : TargetRules
{
	public LyraEditorTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V6;

		Type = TargetType.Editor;
		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });

		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		LyraGameTarget.ApplySharedLyraTargetSettings(this);

		// This is used for touch screen development along with the "Unreal Remote 2" app
		EnablePlugins.Add("RemoteSession");

        // 每次编译在项目根目录运行npm run build
        string projectRoot = ProjectFile.Directory.FullName;
        string tsCmd = $"cd /D \"{projectRoot}\" && npm run build";
        PostBuildSteps.Add(tsCmd);
    }
}
