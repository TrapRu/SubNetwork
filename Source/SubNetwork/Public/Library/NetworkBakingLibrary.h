// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/NetworkManifest.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "NetworkBakingLibrary.generated.h"

// Класс запекания зарезервированных ID устройств
UCLASS()
class SUBNETWORK_API UNetworkBakingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

#if WITH_EDITOR
    // Процесс запекания всех заразервированных ID в один манифест
    // meta=(WorldContext="WorldContextObject") позволяет функции автоматически получить текущий уровень из Blueprints
    UFUNCTION(BlueprintCallable, Category = "Network Editor Tools", meta = (WorldContext = "WorldContextObject"))
    static void BakeNetworkManifest(const UObject* WorldContextObject, UNetworkManifest* TargetManifest);
 
    // Весь процесс запекания выводится в всплывающем окне и сообщает о успешности
    void ShowEditorNotification(const FText& Message, SNotificationItem::ECompletionState State = SNotificationItem::CS_Success)
    {
        FNotificationInfo Info(Message);
        Info.bFireAndForget = true;          // Уведомление исчезнет само
        Info.ExpireDuration = 3.0f;          // Время отображения в секундах
        Info.FadeOutDuration = 0.5f;         // Длительность затухания
        Info.bUseLargeFont = false;          // Обычный размер шрифта

        TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
        if (NotificationItem.IsValid())
        {
            NotificationItem->SetCompletionState(State);
        }
    }
#endif

};
