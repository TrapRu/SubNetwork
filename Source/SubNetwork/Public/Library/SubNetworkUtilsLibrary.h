// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SubNetworkUtilsLibrary.generated.h"

// Вспомогаетельные функции для работы плагина
UCLASS()
class SUBNETWORK_API USubNetworkUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    // Возвращает полученный class в object reference, 
    // для получения стандартных значений/функций этого класса, без необходимости создавать объект и загружать его в память
    UFUNCTION(BlueprintPure, Category = "Core Utilities", meta = (DeterminesOutputType = "ObjectClass"))
    static UObject* GetClassDefaultObject(TSubclassOf<UObject> ObjectClass);

};
