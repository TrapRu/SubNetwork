// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TerminalInterface.generated.h"

class UTerminalCommand;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UTerminalInterface : public UInterface
{
	GENERATED_BODY()
};

// Интерфейс терминала, для управления им и получения другой нужной информации, внутри команды
class SUBNETWORK_API ITerminalInterface
{
	GENERATED_BODY()

public:
	// Функция для чтения словаря команд
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal")
	TMap<FName, TSubclassOf<UTerminalCommand>> GetCommandRegistry() const;

	// Функция для команды clear
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal")
	void ClearTerminalScreen();

	// Функция очистки терминала
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal")
	void CloseTerminal();

	// Функция подключения терминала к устройству
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal")
	void ConnectToDevice(FName DeviceID);

	// Функция отключения терминала от устройства
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal")
	void DisconnectFromDevice();

	// Функция получения DeviceID текущего устройства к которому подключён терминал
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Terminal")
	FName GetConnectedDevice();
};
