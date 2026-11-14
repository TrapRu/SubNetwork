// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SubNetworkSettings.generated.h"

class UNetworkManifest;

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "SubNetwork Settings"))
class SUBNETWORK_API USubNetworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
    USubNetworkSettings()
    {
        // Категория, в которой появится меню в левой панели Project Settings
        CategoryName = TEXT("Game");
    }

    // Мягкая ссылка на манифест. Дизайнер сможет выбрать ассет в выпадающем списке.
    UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Network Generation")
    TSoftObjectPtr<UNetworkManifest> GlobalLevelManifest;

    // Нижняя граница пула рандомных ID
    UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Network Generation")
    int32 RandomPoolMin = 1;

    // Верхняя граница пула рандомных ID
    UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Network Generation")
    int32 RandomPoolMax = 999;

};
