// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/LyraConfigLoader.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString ULyraConfigLoader::GetConfigJsonFileNames()
{
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFiles(FoundFiles, *(GetConfigDir() / TEXT("*.json")), true, false);

	for (FString& FileName : FoundFiles)
	{
		FileName = FPaths::GetBaseFilename(FileName);
	}

	FoundFiles.Sort();
	return FString::Join(FoundFiles, TEXT("|"));
}

FString ULyraConfigLoader::LoadConfigJson(const FString& Name)
{
	const FString FilePath = GetConfigDir() / FString::Printf(TEXT("%s.json"), *Name);
	FString Content;
	if (FFileHelper::LoadFileToString(Content, *FilePath))
	{
		return Content;
	}

	return FString();
}

FString ULyraConfigLoader::GetConfigDir()
{
	return FPaths::ProjectContentDir() / TEXT("AssetsPackage/Config");
}
