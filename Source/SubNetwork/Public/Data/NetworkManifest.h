// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NetworkManifest.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SUBNETWORK_API UNetworkManifest : public UDataAsset
{
	GENERATED_BODY()
	
public:
    // Используем TSet для сверхбыстрого поиска (O(1)) при проверке коллизий
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network")
    TSet<FName> ReservedStaticIDs;

};
