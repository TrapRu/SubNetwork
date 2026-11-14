// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/SubNetworkStruct.h"
#include "Interface/SubNetworkInterface.h"
#include "HackableComponent.generated.h"

class UDeviceActionPreset;

// Динамический делегат с передачей указателя на компонент
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHackableComponentDestroyed, UHackableComponent*, DestroyedComponent);

// Класс компонента устройства
// Позволяет актору компонента взаимодействовать с сетью и быть её частью (реагировать на внешние команды)
UCLASS(Blueprintable, BlueprintType, Abstract, ClassGroup=(Custom))
class SUBNETWORK_API UHackableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Надо ли генерировать ID объекта автоматически
	UPROPERTY(EditAnywhere, Category = "Network|Settings", meta = (AllowPrivateAccess = "true"))
	bool bGenerateIdAtRuntime = true;

	// Если true, устройство физически не существует в мире, а только держит за собой адрес
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Settings", meta = (EditCondition = "!bGenerateIdAtRuntime", EditConditionHides))
	bool bIsReservationProxy = false;

	// Префикс устройства (например, DOOR, CAM, TURRET)
	UPROPERTY(EditAnywhere, Category = "Network|Settings", meta = (EditCondition = "bGenerateIdAtRuntime", EditConditionHides))
	FText DevicePrefix;
	
	// Полное имя устройства, которое будет добавлено в сеть
	UPROPERTY(EditAnywhere, Category = "Network|Settings", meta = (EditCondition = "!bGenerateIdAtRuntime", EditConditionHides))
	FText FullIdentificator;

	// Подсеть в котором находится устройство
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network|Settings", meta = (Categories = "SubNetwork"))
	FGameplayTag SubNetworkTag;

	// Итоговый ID устройства в сети
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Settings", meta = (EditCondition = "false", EditConditionHides = true))
	FName DeviceID;

	// Приветственное сообщение для терминала при подключение
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network|Commands")
	FText WelcomeMassage;

	// Базовый пресет (только для чтения, выбирается в редакторе)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network|Commands")
	TObjectPtr<const UDeviceActionPreset> ActionPreset;

	// Индивидуальные команды конкретно для этого экземпляра (добавляют новые или меняют описание старых)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network|Commands|Delta")
	TMap<FName, FDeviceCommand> AdditionalActions;

	// Команды из пресета, которые нужно отключить на этом экземпляре
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network|Commands|Delta")
	TSet<FName> ExcludedActions;

	// Делегат, доступный для подписки из C++ и Blueprints
	UPROPERTY(BlueprintAssignable, Category = "SubNetwork")
	FOnHackableComponentDestroyed OnDeviceDestroyed;

	// Sets default values for this component's properties
	UHackableComponent();

	// Подключает это устройство в сеть, подсеть выбирается из переменной SubNetworkTag компонента
	UFUNCTION(BlueprintCallable, Category = "SubNetwork", meta = (Categories = "SubNetwork"))
	void ConnectToSubNetwork();

	// Функция вызова команды устройства
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SubNetwork")
	FTerminalResponse ExecuteCommand(FName Command, const TArray<FString>& Args) const;

	virtual void OnRegister() override;

	// Сборка пресета и индивидуальных настроек
	UFUNCTION(BlueprintCallable, Category = "Network|Commands")
	void CompileDeviceActions();

	// Возвращает список команд устройства
	UFUNCTION(BlueprintCallable, Category = "Network|Commands")
	TMap<FName, FDeviceCommand> GetCommadsActions() const;

	// Возвращает все команды подключенного устройства, начинающиеся с префикса
	UFUNCTION(BlueprintCallable, Category = "Network|Commands")
	void GetMatchingActions(const FString& Prefix, TArray<FString>& OutMatches) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

private:
	// Список команд устройства: ключ - вызов в консоле, значение - переменная для свитча
	UPROPERTY(Transient)
	TMap<FName, FDeviceCommand> RuntimeActions;

	void RefreshDeviceID();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};