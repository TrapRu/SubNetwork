// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/SubNetworkStruct.h"
#include "SubNetworkInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class USubNetworkInterface : public UInterface
{
	GENERATED_BODY()
};

// Интефрейс актора устройства, для исполнения полученной команды
class SUBNETWORK_API ISubNetworkInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "SubNetwork")
	FTerminalResponse ExecuteCommand(const FName& Request, const TArray<FString>& Args);
};
