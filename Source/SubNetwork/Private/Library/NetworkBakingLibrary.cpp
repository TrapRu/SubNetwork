// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/NetworkBakingLibrary.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/HackableComponent.h"

#if WITH_EDITOR
bool BakeProcess(const UObject* WorldContextObject, UNetworkManifest* TargetManifest)
{
    bool bError = false;

    if (!TargetManifest)
    {
        UE_LOG(LogTemp, Error, TEXT("Baking failed: Target Manifest is NULL."));
        bError = true;
        return bError;
    }

    // Получаем текущий открытый мир (уровень)
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) 
    {
        UE_LOG(LogTemp, Error, TEXT("Baking failed: Failed to get a link to the world."));
        bError = true;
        return bError;
    }

    TargetManifest->ReservedStaticIDs.Empty();

    // Безопасный проход по всем Actor'ам на загруженном уровне
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (UHackableComponent* HackableComponent = Actor->FindComponentByClass<UHackableComponent>())
        {
            // Пропускаем объекты, которые будут генерироваться динамически, и те, где ID не задан
            if (!HackableComponent->bGenerateIdAtRuntime && HackableComponent->DeviceID != NAME_None)
            {
                if (!TargetManifest->ReservedStaticIDs.Contains(HackableComponent->DeviceID))
                {
                    UE_LOG(LogTemp, Display, TEXT("The Device '%s' has reserved an ID '%s' in the manifest."),
                        *GetNameSafe(HackableComponent ? HackableComponent->GetOwner() : nullptr),
                        *HackableComponent->DeviceID.ToString());

                    TargetManifest->ReservedStaticIDs.Add(HackableComponent->DeviceID);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Network Manifest Bake Warning! The Device '%s' has the same ID '%s' that has already been added to the manifest. A conflict may arise during registration in the SubNetworkSubsystem."),
                        *GetNameSafe(HackableComponent ? HackableComponent->GetOwner() : nullptr),
                        *HackableComponent->DeviceID.ToString());

                    bError = true;
                }
            }
        }
    }

    // Помечаем ассет как измененный, чтобы редактор предложил его сохранить (Звездочка на иконке)
    TargetManifest->MarkPackageDirty();

    return bError;
}

void UNetworkBakingLibrary::BakeNetworkManifest(const UObject* WorldContextObject, UNetworkManifest* TargetManifest)
{
    UE_LOG(LogTemp, Display, TEXT("The process Bake Network Manifest has been launched."));

    // Создаем уведомление с индикатором выполнения
    FNotificationInfo Info(FText::FromString(TEXT("Baking Network Manifest...")));
    Info.bFireAndForget = false; // Не исчезает, пока не завершим вручную
    TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info);
    Item->SetCompletionState(SNotificationItem::CS_Pending);

    // Процесс запекания
    bool bBakeResult = BakeProcess(WorldContextObject, TargetManifest);

    if (!bBakeResult)
    {
        UE_LOG(LogTemp, Display, TEXT("Network Manifest Baked Successfully! Saved %d static device IDs."), TargetManifest->ReservedStaticIDs.Num());

        // После завершения логики:
        Item->SetText(FText::FromString(TEXT("Baking Complete!")));
        Item->SetCompletionState(SNotificationItem::CS_Success);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Network Manifest not Baked correctly!"));

        // После завершения логики:
        Item->SetText(FText::FromString(TEXT("Baking Error! Check Output Log!")));
        Item->SetCompletionState(SNotificationItem::CS_Fail);
    }
    Item->ExpireAndFadeout(); // Запускает таймер затухания
}
#endif