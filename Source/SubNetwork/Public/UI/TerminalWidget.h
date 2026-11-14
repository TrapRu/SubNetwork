// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/SubNetworkStruct.h"
#include "TerminalWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCommandSubmittedDelegate,
	const FString&, Command,
	UTerminalWidget*, Widget
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAutocompleteDelegate,
	const FString&, Input,
	UTerminalWidget*, Widget
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnExitWidgetDelegate
);

// Класс виджета терминала
// Через него игрок взаимодействует с терминалом и в принципе с сетью
UCLASS()
class SUBNETWORK_API UTerminalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Вызывает диспатчер, если была введена команда
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Events")
	FOnCommandSubmittedDelegate OnCommandSubmittedEvent;

	// Вызывает диспатчер, если потребовалось автодополнения ввода в коносль
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Events")
	FOnAutocompleteDelegate OnAutocompleteEvent;

	// Вызывает диспатчер, выйти из терминала
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Events")
	FOnExitWidgetDelegate OnExitWidgetDelegate;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	int32 LogSize = 50;

	// Функция вывода текста в консоль
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void PrintOutput(FTerminalResponse Message);

	// Функция очистки экрана
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void ClearScreen();

	// Функция вывода корня терминала (к чему сейчас подключен терминал)
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void SetRoot(const FText& Root);

	// Функция получения предложений автодополнений ввода в консоль
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void AutocompleteSuggestions(const FAutocompleteResult& Suggestions);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void EnterTerminalFocus(APlayerController* PC);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Terminal")
	void ExitTerminalFocus(APlayerController* PC);
};
