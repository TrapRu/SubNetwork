// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SubNetworkSubsystem.h"
#include "Algo/RandomShuffle.h"
#include "Data/SubNetworkSettings.h"
#include "Components/HackableComponent.h"
#include "Data/NetworkManifest.h"

void USubNetworkSubsystem::AddDevice(FGameplayTag SubNetworkTag, FName DeviceID, UHackableComponent* HackableComponent)
{
    if (!IsValid(HackableComponent))
    {
        return;
    }

    if (DeviceID == NAME_None || DeviceID == "None") 
    {
        return;
    }

    // Подписываемся на диспатчер уничтожения устройства, что бы можно было его убрать из сети в случае уничтожения
    HackableComponent->OnDeviceDestroyed.AddUniqueDynamic(this, &USubNetworkSubsystem::HandleDeviceDestroyed);

    FDevicesMapWrapper& Wrapper = ChildSubNetwork.FindOrAdd(SubNetworkTag);

    Wrapper.DevicesMap.Emplace(DeviceID, HackableComponent);

}

UHackableComponent* USubNetworkSubsystem::GetDevice(FGameplayTag SubNetworkTag, FName DeviceID)
{
    if (const FDevicesMapWrapper* FoundWrapper = ChildSubNetwork.Find(SubNetworkTag))
    {
        if (const TWeakObjectPtr<UHackableComponent>* WeakPtr = FoundWrapper -> DevicesMap.Find(DeviceID))
        {
            return WeakPtr -> Get();
        }
    }

    return nullptr;
}

void USubNetworkSubsystem::HandleDeviceDestroyed(UHackableComponent* DestroyedComponent)
{
    if (IsValid(DestroyedComponent))
    {
        RemoveDevice(DestroyedComponent->SubNetworkTag, DestroyedComponent->DeviceID);
    }
}

void USubNetworkSubsystem::RemoveDevice(FGameplayTag SubNetworkTag, FName DeviceID)
{
    if (FDevicesMapWrapper* FoundWrapper = ChildSubNetwork.Find(SubNetworkTag))
    {
        FoundWrapper -> DevicesMap.Remove(DeviceID);
    }
}

TArray<FName> USubNetworkSubsystem::GetKeys(FGameplayTag SubNetworkTag) const
{
	TArray<FName> KeyArray;

    if (const FDevicesMapWrapper* FoundWrapper = ChildSubNetwork.Find(SubNetworkTag))
    {
        KeyArray.Reserve(FoundWrapper->DevicesMap.Num());

        for (const auto& Kvp : FoundWrapper->DevicesMap)
        {
            KeyArray.Add(Kvp.Key);
        }
    }

	return KeyArray;
}

TArray<UHackableComponent*> USubNetworkSubsystem::GetValues(FGameplayTag SubNetworkTag) const
{
    TArray<UHackableComponent*> ValueArray;

    if (const FDevicesMapWrapper* FoundWrapper = ChildSubNetwork.Find(SubNetworkTag))
    {
        ValueArray.Reserve(FoundWrapper->DevicesMap.Num());

        for (const auto& Kvp : FoundWrapper->DevicesMap)
        {
            // Kvp.Value — const TWeakObjectPtr<UHackableComponent>&
            // Дополнительная валидация на случай задержки тика сборщика мусора
            if (UHackableComponent* Comp = Kvp.Value.Get())
            {
                ValueArray.Add(Comp);
            }
        }
    }

	return ValueArray;
}

FName USubNetworkSubsystem::AllocateRandomUnusedID(const FString& Prefix)
{
    // 1. Получаем доступ к глобальным настройкам из Project Settings
    const USubNetworkSettings* Settings = GetDefault<USubNetworkSettings>();
    // Находим пул для данного префикса или создаем пустой
    TArray<int32>& Pool = PrefixPools.FindOrAdd(Prefix);

    // Если пул пуст, значит это первый запрос для этого типа устройств. 
    // Заполняем "мешок".
    if (Pool.Num() == 0)
    {
        // Допустим, мы хотим трехзначные суффиксы
        for (int32 i = Settings->RandomPoolMin; i <= Settings->RandomPoolMax; ++i)
        {
            bool bFlag = true;

            // Собираем проверочную строку
            FString TestString = FString::Printf(TEXT("%s_%d"), *Prefix, i);

            // ПРОАКТИВНЫЙ КОНТРОЛЬ:
            // Пытаемся найти имя в глобальной таблице. НЕ создаем новое!
            FName TestID = FName(*TestString, FNAME_Find);

            // Если имени не существует в глобальной таблице, значит левел-дизайнер 
            // его не использовал и в манифест оно не попало. Слот гарантированно свободен.
            if (TestID == NAME_None)
            {
                Pool.Add(i);
                continue; // Переходим к следующему числу, пропуская проверки ниже
            }

            // Если код дошел сюда, значит такое имя уже когда-то создавалось в памяти.
            // Теперь нужно убедиться, занято ли оно именно на НАШЕМ уровне, 
            // или оно просто закешировалось от других систем/уровней.
            bool bIsManifestReserved = LevelManifest && LevelManifest->ReservedStaticIDs.Contains(TestID);

            // Если этот ID был зарезервирован - пропускаем
            if (bIsManifestReserved)
            {
                continue;
            }

            // Если сабсистема пуста, то ни одно устройство не было добавлено - все ID свободны
            if (ChildSubNetwork.IsEmpty())
            {
                Pool.Add(i);
                continue;
            }

            // Проход по всем подсетям
            for (const auto& Pair : ChildSubNetwork)
            {
                const FDevicesMapWrapper& Value = Pair.Value;

                // Если такой ID был найден в сабсистеме - его нельзя использовать
                if (Value.DevicesMap.Contains(TestID))
                {
                    bFlag = false;
                    break;
                }
            }

            // Если все проверки пройдены - добавляем пул
            if (bFlag) {
                Pool.Add(i);
            }
        }

        // Перемешиваем элементы массива случайным образом
        Algo::RandomShuffle(Pool);
    }

    // Защита от полного исчерпания диапазона
    if (Pool.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("NETWORK ERROR: Out of available IDs for prefix %s"), *Prefix);
        return NAME_None;
    }

    // Вынимаем последнее случайное число из мешка (O(1))
    int32 RandomSuffix = Pool.Pop();

    // Формируем итоговый идентификатор
    FString NewIDString = FString::Printf(TEXT("%s_%d"), *Prefix, RandomSuffix);
    return FName(*NewIDString);
}

void USubNetworkSubsystem::GetMatchingDeviceIDs(const FString& Prefix, FGameplayTag SubNetworkTag, TArray<FString>& OutMatches) const
{
    if (const FDevicesMapWrapper* Wrapper = ChildSubNetwork.Find(SubNetworkTag))
    {
        for (const TPair<FName, TWeakObjectPtr<UHackableComponent>>& Pair : Wrapper->DevicesMap)
        {
            if (Pair.Value.IsValid() && !Pair.Key.IsNone())
            {
                FString DeviceIDStr = Pair.Key.ToString();
                if (DeviceIDStr.StartsWith(Prefix, ESearchCase::IgnoreCase))
                {
                    OutMatches.Add(DeviceIDStr);
                }
            }
        }
    }
}

void USubNetworkSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    // 1. Получаем доступ к глобальным настройкам из Project Settings
    const USubNetworkSettings* Settings = GetDefault<USubNetworkSettings>();

    // 2. Проверяем, назначен ли манифест в настройках
    if (!Settings->GlobalLevelManifest.IsNull())
    {
        // 3. Синхронно загружаем DataAsset в оперативную память
        // LoadSynchronous() допустим здесь, так как DataAsset (TSet<FName>) весит несколько байт
        LevelManifest = Settings->GlobalLevelManifest.LoadSynchronous();

        if (!LevelManifest)
        {
            UE_LOG(LogTemp, Error, TEXT("Network Subsystem failed to load GlobalLevelManifest from settings!"));
        }
    }
}
