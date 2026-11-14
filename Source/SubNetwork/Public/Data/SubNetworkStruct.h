// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubNetworkStruct.generated.h"


UENUM(BlueprintType)
enum class ETerminalLogStyle : uint8
{
    Info        UMETA(DisplayName = "Обычная информация"),
    Success     UMETA(DisplayName = "Успех"),
    Warning     UMETA(DisplayName = "Предупреждение"),
    Error       UMETA(DisplayName = "Ошибка"),
    System      UMETA(DisplayName = "Системное сообщение")
};

// Структура команды устройства
USTRUCT(BlueprintType)
struct SUBNETWORK_API FDeviceCommand
{
    GENERATED_BODY()

    // Название команды для switch
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CommandName;

    // Описание команды
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    // Цвет команды (использовать по усмотрению)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DescriptionTextColor = FLinearColor::White;
};

// Структура ответа на запрос терминала
USTRUCT(BlueprintType)
struct SUBNETWORK_API FTerminalResponse
{
    GENERATED_BODY()

    // Текст, который будет выведен в консоль
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText OutputText;

    // Цвет текста
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerminalLogStyle TextStyle = ETerminalLogStyle::Info;

    // Флаг успеха
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSuccess = true;

    // Номер сообщения в терминале (изменяется в TerminalComponent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MessageID;
};

// Структура автодополнения ввода в консоль
USTRUCT(BlueprintType)
struct FAutocompleteResult
{
    GENERATED_BODY()

    // Готовая строка, которую виджет должен вставить в EditableTextBox
    UPROPERTY(BlueprintReadOnly)
    FString CompletedInput;

    // Список вариантов (если совпадений больше одного, UI может вывести их в лог)
    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Suggestions;
};