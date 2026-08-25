// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LyraConfigLoader.generated.h"

UCLASS()
class LYRAGAME_API ULyraConfigLoader : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lyra|Config")
	static FString GetConfigJsonFileNames();

	UFUNCTION(BlueprintCallable, Category = "Lyra|Config")
	static FString LoadConfigJson(const FString& Name);

private:
	static FString GetConfigDir();
};
