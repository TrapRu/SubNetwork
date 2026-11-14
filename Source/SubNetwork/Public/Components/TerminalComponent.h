// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objects/TerminalCommand.h"
#include "Data/TerminalPresets.h"
#include "TerminalComponent.generated.h"

class UTerminalWidget;
class UTerminalCommandPreset;
class USubNetworkSubsystem;

// Класс компонент терминала
// Обрабатывает запросы и вызывает команды внутри своей подсети
UCLASS(Blueprintable, BlueprintType, Abstract, ClassGroup=(Custom))
class SUBNETWORK_API UTerminalComponent : public UActorComponent, public ITerminalInterface
{
	GENERATED_BODY()

public:
	// В какой подсети находится терминал (указывается геймплей тег SubNetwork)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminal", meta = (Categories = "SubNetwork"))
	FGameplayTag NetworkTag;

	// Базовый пресет команд терминала
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal|Config")
	TObjectPtr<const UTerminalCommandPreset> CommandPreset;

	// Исключенные команды (удаляются как из Global, так и из Local)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminal|Config|Delta")
	TSet<FName> ExcludedCommands;

	// Добавленные или измененные команды для этого терминала
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminal|Config|Delta")
	TMap<FName, FTerminalCommandOverride> AdditionalCommands;

	// Позволяет подменить прослойку связи с устройством для конкретного терминала (если не задано — берется из пресета)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminal|Config|Delta")
	TSubclassOf<UTerminalCommand> OverrideDeviceProxyCommand;

	// К какому DeviceID подключён терминал сейчас
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedDevice, BlueprintReadWrite, Category = "Terminal")
	FName ConnectedDevice = NAME_None;

	// Sets default values for this component's properties
	UTerminalComponent();

	// Инициализация компонента терминала
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void Init(UTerminalWidget* Widget);

	// Функция обработки запроса и возращение ответа
	UFUNCTION(BlueprintCallable, Category = "Terminal", meta = (Categories = "SubNetwork"))
	FTerminalResponse ProcessInput(const FString& RawInput, const FTerminalContext& Context);

	// Функция обработки запроса от виджета
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void ResponseRequest(const FString& RawInput, UTerminalWidget* Widget);

	// Функция сборки всех команд терминала
	UFUNCTION(BlueprintCallable, Category = "Terminal|Config")
	void CompileTerminalCommands();

	// Функция автодополнения запроса
	// Из полученного текстового ввода, собирает совпадения и вычисляет наибольший общий префикс(Longest Common Prefix), 
	// как это работает в стандартных оболочках вроде Bash, и выводит их в виде структуры AutocompleteResult.
	UFUNCTION(BlueprintCallable, Category = "Terminal|Autocomplete")
	FAutocompleteResult ProcessAutocomplete(const FString& RawInput, const FTerminalContext& Context);

	// Функции интерфейса
	virtual TMap<FName, TSubclassOf<UTerminalCommand>> GetCommandRegistry_Implementation() const override;
	virtual void ClearTerminalScreen_Implementation() override;
	virtual void CloseTerminal_Implementation() override;
	virtual void ConnectToDevice_Implementation(FName DeviceID) override;
	virtual void DisconnectFromDevice_Implementation() override;
	virtual FName GetConnectedDevice_Implementation() override;

private:
	// Скомпилированные рантайм-данные
	UPROPERTY(Transient)
	TMap<FName, TSubclassOf<UTerminalCommand>> RuntimeGlobalCommands;

	UPROPERTY(Transient)
	TMap<FName, TSubclassOf<UTerminalCommand>> RuntimeLocalCommands;

	UPROPERTY(Transient)
	TSubclassOf<UTerminalCommand> RuntimeDeviceProxyCommand;

protected:
	// Переопределение метода для регистрации реплицируемых переменных
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal|Replication")
	void OnRep_ConnectedDevice();
};
