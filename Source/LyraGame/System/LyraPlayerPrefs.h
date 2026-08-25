// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LyraPlayerPrefs.generated.h"

UCLASS()
class LYRAGAME_API ULyraPlayerPrefsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;

	UPROPERTY()
	TMap<FString, FString> StringData;

	UPROPERTY()
	TMap<FString, int32> IntData;
};

UCLASS()
class LYRAGAME_API ULyraPlayerPrefs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static ULyraPlayerPrefsSaveGame *LoadSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static FString GetString(const FString &Key, const FString &DefaultValue);

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static int32 GetInt(const FString &Key, int32 DefaultValue);

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static void SetString(const FString &Key, const FString &Value);

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static void SetInt(const FString &Key, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static void Delete(const FString &Key);

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerPrefs")
	static void SaveData();

private:
	static ULyraPlayerPrefsSaveGame *GetOrCreateSaveGame();

	static TObjectPtr<ULyraPlayerPrefsSaveGame> Data;
};
