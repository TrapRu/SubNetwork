// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/TerminalCommand.h"

#define LOCTEXT_NAMESPACE "SubNetwork"

FTerminalResponse UTerminalCommand::ValidateInput_Implementation(const TArray<FString>& Args, const FTerminalContext& Context) const
{
    FTerminalResponse Response;
    Response.bIsSuccess = true;

    // Базовая проверка на количество аргументов
    if (Args.Num() < 1)
    {
        Response.OutputText = FText::FromString(TEXT("ERROR: Missing required arguments."));
        Response.TextStyle = ETerminalLogStyle::Error;
        Response.bIsSuccess = false;
    }

    if (!Context.NetworkSubsystem)
    {
        Response.OutputText = FText::FromString(TEXT("ERROR: Terminal not connected to network."));
        Response.TextStyle = ETerminalLogStyle::Error;
        Response.bIsSuccess = false;
    }

    if (!Context.SubNetworkTag.IsValid())
    {
        Response.OutputText = FText::FromString(TEXT("ERROR: Terminal not connected to network."));
        Response.TextStyle = ETerminalLogStyle::Error;
        Response.bIsSuccess = false;
    }

    return Response;
}

FTerminalResponse UTerminalCommand::PerformCommandLogic_Implementation(const TArray<FString>& Args, const FTerminalContext& Context) const
{
    FTerminalResponse Response;
    // Логика команды по умолчанию
    return Response;
}

FTerminalResponse UTerminalCommand::PerformDeviceLogic_Implementation(const TArray<FString>& Args, const FTerminalContext& Context) const
{
    FTerminalResponse Response;
    // Логика команды по умолчанию
    return Response;
}

FTerminalResponse UTerminalCommand::ExecuteCommand(const TArray<FString>& Args, const FTerminalContext& Context) const
{
    // Сначала запускаем валидацию
    FTerminalResponse ValidationResponse = ValidateInput(Args, Context);

    // Если валидация провалена, сразу возвращаем ошибку в UI
    if (!ValidationResponse.bIsSuccess)
    {
        return ValidationResponse;
    }

    if (ITerminalInterface::Execute_GetConnectedDevice(Context.TerminalInterface.GetObject()) != NAME_None && SeparateDeviceLogic)
    {
        return PerformDeviceLogic(Args, Context);
    }

    // Выполняем полезную нагрузку
    return PerformCommandLogic(Args, Context);
}

TArray<FString> UTerminalCommand::GetSuggestions_Implementation(
    FName CommandName, bool bIsCompletingCommand, int32 ArgIndex,
    const FString& CurrentPrefix, const FTerminalContext& Context) const
{
    TArray<FString> DeviceSuggestions;
    return DeviceSuggestions;
}

#undef LOCTEXT_NAMESPACE