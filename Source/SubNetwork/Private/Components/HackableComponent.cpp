// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HackableComponent.h"
#include "Subsystems/SubNetworkSubsystem.h"
#include "Tags/SubNetworkTags.h"
#include "Data/TerminalPresets.h"

UHackableComponent::UHackableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	SubNetworkTag = SubNetworkTags::Tag_SubNetwork;
}

void UHackableComponent::ConnectToSubNetwork()
{
	USubNetworkSubsystem* NetworkSubsystem = GetWorld()->GetSubsystem<USubNetworkSubsystem>();

	if (IsValid(NetworkSubsystem) && !(bIsReservationProxy && !bGenerateIdAtRuntime)) {
		NetworkSubsystem->AddDevice(SubNetworkTag, DeviceID, this);
	}
}

FTerminalResponse UHackableComponent::ExecuteCommand_Implementation(FName Command, const TArray<FString>& Args) const
{
	FTerminalResponse Response;
	return Response;
}

void UHackableComponent::BeginPlay()
{
	// Если ID нужно генерировать автоматически
	if (!DevicePrefix.IsEmpty() && bGenerateIdAtRuntime)
	{
		USubNetworkSubsystem* NetworkSubsystem = GetWorld()->GetSubsystem<USubNetworkSubsystem>();

		if (IsValid(NetworkSubsystem)) {
			DeviceID = NetworkSubsystem->AllocateRandomUnusedID(DevicePrefix.ToString());
		}
	}

	Super::BeginPlay();
}

void UHackableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Оповещаем подсистему о том, что устройство уходит из сети
	if (OnDeviceDestroyed.IsBound())
	{
		OnDeviceDestroyed.Broadcast(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UHackableComponent::RefreshDeviceID()
{
	const FName NewDeviceID = FName(*FullIdentificator.ToString());

	if (DeviceID != NewDeviceID)
	{
		// Помечаем объект модифицированным для транзакции уровня
		Modify();

		DeviceID = NewDeviceID;

		UE_LOG(LogTemp, Warning, TEXT("[Instance: %s] DeviceID updated to: %s"), *GetName(), *DeviceID.ToString());
	}
}

void UHackableComponent::OnRegister()
{
	Super::OnRegister();

	// OnRegister гарантированно вызывается на живом инстансе в мире после любого RerunConstructionScripts
	RefreshDeviceID();
}

void UHackableComponent::CompileDeviceActions()
{
	RuntimeActions.Empty();

	// 1. Загружаем базу из DataAsset
	if (ActionPreset)
	{
		RuntimeActions = ActionPreset->SupportedActions;
	}

	// 2. Удаляем исключенные геймдизайнером команды
	for (const FName& ExcludedKey : ExcludedActions)
	{
		RuntimeActions.Remove(ExcludedKey);
	}

	// 3. Добавляем уникальные команды экземпляра (или перезаписываем описание существующих)
	for (const TPair<FName, FDeviceCommand>& Pair : AdditionalActions)
	{
		RuntimeActions.Add(Pair.Key, Pair.Value);
	}
}

TMap<FName, FDeviceCommand> UHackableComponent::GetCommadsActions() const
{
	return RuntimeActions;
}

void UHackableComponent::GetMatchingActions(const FString& Prefix, TArray<FString>& OutMatches) const
{
	for (const TPair<FName, FDeviceCommand>& Pair : RuntimeActions)
	{
		FString ActionStr = Pair.Key.ToString();
		if (ActionStr.StartsWith(Prefix, ESearchCase::IgnoreCase))
		{
			OutMatches.Add(ActionStr);
		}
	}
}

#if WITH_EDITOR
void UHackableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	const FName TargetName = GET_MEMBER_NAME_CHECKED(UHackableComponent, FullIdentificator);

	if (PropName == TargetName || MemberPropName == TargetName)
	{
		RefreshDeviceID();

		// Если редактировался инстанс на карте, помечаем актера грязным для сохранения на уровне
		if (AActor* MyOwner = GetOwner())
		{
			MyOwner->Modify();
		}
	}
}

void UHackableComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	const FName TargetName = GET_MEMBER_NAME_CHECKED(UHackableComponent, FullIdentificator);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == TargetName)
	{
		RefreshDeviceID();
	}
}
#endif