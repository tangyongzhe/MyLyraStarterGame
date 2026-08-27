// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraClientTarget : TargetRules
{
	public LyraClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;

		ExtraModuleNames.AddRange(new string[] { "LyraGame" });

		LyraGameTarget.ApplySharedLyraTargetSettings(this);

        // 每次编译在项目根目录运行npm run build
        string projectRoot = ProjectFile.Directory.FullName;
        string tsCmd = $"cd /D \"{projectRoot}\" && npm run build";
        PostBuildSteps.Add(tsCmd);
    }
}
