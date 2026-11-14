// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "SubNetworkSubsystem.generated.h"

class UHackableComponent;
class UNetworkManifest;

// Класс сабсистемы подсети
// Через эту сабсистему все устройства в сети могут между собой взаимодействовать
UCLASS()
class SUBNETWORK_API USubNetworkSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

// Все переменные сети хранятся в закрытую, никто не может управлять ей напрямую, только через предоставленные функции
private:
	struct FDevicesMapWrapper
	{
		// Все устройства добавленные в сеть хрянятся в этой TMap
		// Ключ FName является DeviceID устройства
		// Значение это ссылка на само устройство
		TMap<FName, TWeakObjectPtr<UHackableComponent>> DevicesMap;
	};

	// TMap подсетей, подсеть указывается геймплейтегом SubNetworkTag
	TMap<FGameplayTag, FDevicesMapWrapper> ChildSubNetwork;

	// Хранит свободные ID под каждый различный префикс устройств
	TMap<FString, TArray<int32>> PrefixPools;

	// Манифест уровня, где хранятся зарезервированные ID устройств
	UPROPERTY()
	UNetworkManifest* LevelManifest = nullptr;

	// Функция автоудаления устройства из сети в случае его уничтожения
	UFUNCTION()
	void HandleDeviceDestroyed(UHackableComponent* DestroyedComponent);

public:
	// Функция добавления устройства в сеть
	// SubNetworkTag - геймплей тег подсети
	// DeviceID - уникальный идентификатор устройства
	// HackableComponent - ссылка на сам компонент устройства
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	void AddDevice(FGameplayTag SubNetworkTag, FName DeviceID, UHackableComponent* HackableComponent);

	// Функция получения ссылки на устройство в сети
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	UHackableComponent* GetDevice(FGameplayTag SubNetworkTag, FName DeviceID);

	// Функция удаления устройства из сети
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	void RemoveDevice(FGameplayTag SubNetworkTag, FName DeviceID);
	
	// Функция получения всех DeviceID устройств в сети
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	TArray<FName> GetKeys(FGameplayTag SubNetworkTag) const;

	// Функция получения всех ссылок на устройства в сети
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	TArray<UHackableComponent*> GetValues(FGameplayTag SubNetworkTag) const;

	// Функция получения свободного ID для устройства
	UFUNCTION(BlueprintCallable, Category = "SubNetwork")
	FName AllocateRandomUnusedID(const FString& Prefix);

	// Возвращает все DeviceID в сети, начинающиеся с введенного префикса
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	void GetMatchingDeviceIDs(const FString& Prefix, FGameplayTag SubNetworkTag, TArray<FString>& OutMatches) const;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
