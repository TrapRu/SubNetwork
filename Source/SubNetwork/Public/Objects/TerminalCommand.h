// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Data/SubNetworkStruct.h"
#include "Interface/TerminalInterface.h"
#include "TerminalCommand.generated.h"

class USubNetworkSubsystem;

// Структура контекста терминала
USTRUCT(BlueprintType)
struct FTerminalContext
{
    GENERATED_BODY()

    // Указатель на подсистему сети (для сетевых команд)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USubNetworkSubsystem* NetworkSubsystem = nullptr;

    // В какой подсети находится сейчас терминал
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "SubNetwork"))
    FGameplayTag SubNetworkTag;

    // Безопасный интерфейс терминала (для локальных команд)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TScriptInterface<ITerminalInterface> TerminalInterface;
};

// Класс команды терминала
// Вся логика команд прописывается в этом классе
UCLASS(Blueprintable, Abstract)
class SUBNETWORK_API UTerminalCommand : public UObject
{
    GENERATED_BODY()

protected:
    // Метод валидации. Можно переопределять.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal|Validation", meta = (Categories = "SubNetwork"))
    FTerminalResponse ValidateInput(const TArray<FString>& Args, const FTerminalContext& Context) const;

    // Метод логики. Срабатывает, если терминал НЕ подключён к устройству. Можно переопределять.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal|Logic", meta = (Categories = "SubNetwork"))
    FTerminalResponse PerformCommandLogic(const TArray<FString>& Args, const FTerminalContext& Context) const;

    // Метод логики устройства. Срабатывает, если терминал подключён к устройству. Можно переопределять.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal|Logic", meta = (Categories = "SubNetwork"))
    FTerminalResponse PerformDeviceLogic(const TArray<FString>& Args, const FTerminalContext& Context) const;

public:
    // Описание команды
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    FText Description;

    // Должна ли эта команда выполнять другую логику (PerformDeviceLogic), если терминал подключён к устройству
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    bool SeparateDeviceLogic = false;

    //Функция вызова команды
    UFUNCTION(BlueprintCallable, Category = "Terminal", meta = (Categories = "SubNetwork"))
    FTerminalResponse ExecuteCommand(const TArray<FString>& Args, const FTerminalContext& Context) const;

    // Метод получения вариантов автодополнения.
    // bIsCompletingCommand = true, если прослойку просят дополнить саму команду устройства (первое слово).
    // bIsCompletingCommand = false, если дополняется аргумент уже введенной команды.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal|Autocomplete")
    TArray<FString> GetSuggestions(
        FName CommandName,
        bool bIsCompletingCommand,
        int32 ArgIndex,
        const FString& CurrentPrefix,
        const FTerminalContext& Context) const;

};
