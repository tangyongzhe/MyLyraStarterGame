// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/LyraPlayerPrefs.h"

#include "Kismet/GameplayStatics.h"

const FString ULyraPlayerPrefsSaveGame::SlotName = TEXT("LyraPlayerPrefs");
TObjectPtr<ULyraPlayerPrefsSaveGame> ULyraPlayerPrefs::Data = nullptr;

ULyraPlayerPrefsSaveGame *ULyraPlayerPrefs::LoadSaveGame()
{
	return GetOrCreateSaveGame();
}

FString ULyraPlayerPrefs::GetString(const FString &Key, const FString &DefaultValue)
{
	ULyraPlayerPrefsSaveGame *SaveGame = GetOrCreateSaveGame();
	if (!SaveGame)
	{
		return DefaultValue;
	}

	if (const FString *FoundValue = SaveGame->StringData.Find(Key))
	{
		return *FoundValue;
	}

	return DefaultValue;
}

int32 ULyraPlayerPrefs::GetInt(const FString &Key, int32 DefaultValue)
{
	ULyraPlayerPrefsSaveGame *SaveGame = GetOrCreateSaveGame();
	if (!SaveGame)
	{
		return DefaultValue;
	}

	if (const int32 *FoundValue = SaveGame->IntData.Find(Key))
	{
		return *FoundValue;
	}

	return DefaultValue;
}

void ULyraPlayerPrefs::SetString(const FString &Key, const FString &Value)
{
	if (ULyraPlayerPrefsSaveGame *SaveGame = GetOrCreateSaveGame())
	{
		SaveGame->StringData.Add(Key, Value);
		SaveData();
	}
}

void ULyraPlayerPrefs::SetInt(const FString &Key, int32 Value)
{
	if (ULyraPlayerPrefsSaveGame *SaveGame = GetOrCreateSaveGame())
	{
		SaveGame->IntData.Add(Key, Value);
		SaveData();
	}
}

void ULyraPlayerPrefs::Delete(const FString &Key)
{
	if (ULyraPlayerPrefsSaveGame *SaveGame = GetOrCreateSaveGame())
	{
		SaveGame->StringData.Remove(Key);
		SaveGame->IntData.Remove(Key);
		SaveData();
	}
}

ULyraPlayerPrefsSaveGame *ULyraPlayerPrefs::GetOrCreateSaveGame()
{
	if (Data)
	{
		if (IsValid(Data) && Data->IsA<ULyraPlayerPrefsSaveGame>())
		{
			return Data;
		}

		Data = nullptr;
	}

	if (USaveGame *LoadedSaveGame = UGameplayStatics::LoadGameFromSlot(ULyraPlayerPrefsSaveGame::SlotName, 0))
	{
		if (ULyraPlayerPrefsSaveGame *LoadedPrefs = Cast<ULyraPlayerPrefsSaveGame>(LoadedSaveGame))
		{
			Data = LoadedPrefs;
			return Data;
		}
		
		Data = nullptr;
	}

	Data = Cast<ULyraPlayerPrefsSaveGame>(UGameplayStatics::CreateSaveGameObject(ULyraPlayerPrefsSaveGame::StaticClass()));
	return Data;
}

void ULyraPlayerPrefs::SaveData()
{
	if (Data)
	{
		UGameplayStatics::SaveGameToSlot(Data, ULyraPlayerPrefsSaveGame::SlotName, 0);
	}
}
