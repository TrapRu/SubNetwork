// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/SubNetworkStruct.h"
#include "TerminalPresets.generated.h"

class UTerminalCommand;

/**
 * 
 */
UENUM(BlueprintType)
enum class ETerminalCommandScope : uint8
{
    // Выполняется только когда терминал НЕ подключен к устройству (connect, list)
    Local UMETA(DisplayName = "Local (Disconnected Only)"),

    // Выполняется всегда первым приоритетом (help, exit, disconnect, clear)
    Global UMETA(DisplayName = "Global (Always Active)")
};

// Структура перезаписывания команды терминала
USTRUCT(BlueprintType)
struct FTerminalCommandOverride
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerminalCommandScope Scope = ETerminalCommandScope::Local;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UTerminalCommand> CommandClass;
};

// Дата ассет пресета команды терминала
UCLASS(BlueprintType)
class UTerminalCommandPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    // 1. Высший приоритет: работают всегда
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Global Commands")
    TMap<FName, TSubclassOf<UTerminalCommand>> GlobalCommands;

    // 2. Команда-прослойка: вызывается, если терминал подключен к устройству
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "2. Device Proxy")
    TSubclassOf<UTerminalCommand> DeviceProxyCommand;

    // 3. Обычный приоритет: работают только без активного подключения
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "3. Local Commands")
    TMap<FName, TSubclassOf<UTerminalCommand>> LocalCommands;
};

// Дата ассет пресета команды устройства
UCLASS(BlueprintType)
class SUBNETWORK_API UDeviceActionPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Ключ: Имя команды (например, "open"). Значение: Описание для консоли ("Opens the magnetic lock")
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Commands")
    TMap<FName, FDeviceCommand> SupportedActions;
};