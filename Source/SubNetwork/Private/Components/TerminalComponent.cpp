// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TerminalComponent.h"
#include "Net/UnrealNetwork.h"

UTerminalComponent::UTerminalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UTerminalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Регистрируем переменную для синхронизации по сети
    DOREPLIFETIME(UTerminalComponent, ConnectedDevice);
}

FTerminalResponse UTerminalComponent::ProcessInput(const FString& RawInput, const FTerminalContext& Context)
{
    FTerminalResponse Response;

    // Очистка строки от лишних пробелов по краям и двойных пробелов внутри
    FString CleanInput = RawInput.TrimStartAndEnd();
    CleanInput = CleanInput.Replace(TEXT("  "), TEXT(" "));

    if (CleanInput.IsEmpty()) return Response;

    // Разбивка строки на массив
    TArray<FString> ParsedWords;
    CleanInput.ParseIntoArray(ParsedWords, TEXT(" "), true);

    // Извлекаем строковое представление команды для возможного вывода в тексте ошибки
    FString CommandKeyStr = ParsedWords[0];
    ParsedWords.RemoveAt(0); // В ParsedWords остаются только FString-аргументы

    // Конвертируем FString в FName через FNAME_Find (без засорения NamePool)
    FName CommandName(*CommandKeyStr, FNAME_Find);

    // Если такого FName вообще нет в таблице движка — ни один пресет или компонент его не регистрировал
    if (CommandName.IsNone())
    {
        Response.OutputText = FText::FromString(FString::Printf(TEXT("COMMAND NOT FOUND: '%s'. Type 'help' for available commands."), *CommandKeyStr));
        Response.TextStyle = ETerminalLogStyle::Error;
        Response.bIsSuccess = false;
        return Response;
    }

    // Выполняем основные команды терминала
    if (TSubclassOf<UTerminalCommand>* CommandClassPtr = RuntimeGlobalCommands.Find(CommandName))
    {
        if (UClass* CommandClass = *CommandClassPtr)
        {
            // Получаем CDO (Class Default Object) класса команды. 
            // Это избавляет от необходимости спавнить объект (NewObject) при каждом вводе.
            const UTerminalCommand* CommandCDO = CommandClass->GetDefaultObject<UTerminalCommand>();

            // Вызываем логику и возвращаем результат в UI
            return CommandCDO->ExecuteCommand(ParsedWords, Context);
        }
    }
    // Если терминал подключён к устройству - выполняем его логику
    if (ConnectedDevice != NAME_None && IsValid(RuntimeDeviceProxyCommand))
    {
        if (UClass* CommandClass = *RuntimeDeviceProxyCommand)
        {
            // Получаем CDO (Class Default Object) класса команды. 
            // Это избавляет нас от необходимости спавнить объект (NewObject) при каждом вводе.
            const UTerminalCommand* CommandCDO = CommandClass->GetDefaultObject<UTerminalCommand>();

            TArray<FString> Request;
            CleanInput.ParseIntoArray(Request, TEXT(" "), true);

            // Вызываем логику и возвращаем результат в UI
            return CommandCDO->ExecuteCommand(Request, Context);
        }
    }
    // Выполняем прочие команды терминала
    if (TSubclassOf<UTerminalCommand>* CommandClassPtr = RuntimeLocalCommands.Find(CommandName))
    {
        if (UClass* CommandClass = *CommandClassPtr)
        {
            // Получаем CDO (Class Default Object) класса команды. 
            // Это избавляет нас от необходимости спавнить объект (NewObject) при каждом вводе.
            const UTerminalCommand* CommandCDO = CommandClass->GetDefaultObject<UTerminalCommand>();

            // Вызываем логику и возвращаем результат в UI
            return CommandCDO->ExecuteCommand(ParsedWords, Context);
        }
    }

    // Если такой команды нет в терминале - выдаём соответствующий ответ
    Response.OutputText = FText::FromString(FString::Printf(TEXT("COMMAND NOT FOUND: '%s'. Type 'help' for available commands."), *CommandKeyStr));
    Response.TextStyle = ETerminalLogStyle::Error;
    Response.bIsSuccess = false;
    return Response;
}

void UTerminalComponent::CompileTerminalCommands()
{
    RuntimeGlobalCommands.Empty();
    RuntimeLocalCommands.Empty();
    RuntimeDeviceProxyCommand = nullptr;

    // 1. Загружаем базу из пресета
    if (CommandPreset)
    {
        for (const TPair<FName, TSubclassOf<UTerminalCommand>>& Pair : CommandPreset->GlobalCommands)
        {
            if (!Pair.Key.IsNone() && Pair.Value)
            {
                RuntimeGlobalCommands.Add(Pair.Key, Pair.Value);
            }
        }

        for (const TPair<FName, TSubclassOf<UTerminalCommand>>& Pair : CommandPreset->LocalCommands)
        {
            if (!Pair.Key.IsNone() && Pair.Value && !RuntimeGlobalCommands.Contains(Pair.Key))
            {
                RuntimeLocalCommands.Add(Pair.Key, Pair.Value);
            }
        }

        RuntimeDeviceProxyCommand = CommandPreset->DeviceProxyCommand;
    }

    // 2. Удаляем исключенные команды
    for (const FName& ExcludedKey : ExcludedCommands)
    {
        RuntimeGlobalCommands.Remove(ExcludedKey);
        RuntimeLocalCommands.Remove(ExcludedKey);
    }

    // 3. Применяем дополнительные команды со сквозной очисткой
    for (const TPair<FName, FTerminalCommandOverride>& Pair : AdditionalCommands)
    {
        if (Pair.Key.IsNone() || !Pair.Value.CommandClass) continue;

        RuntimeGlobalCommands.Remove(Pair.Key);
        RuntimeLocalCommands.Remove(Pair.Key);

        if (Pair.Value.Scope == ETerminalCommandScope::Global)
        {
            RuntimeGlobalCommands.Add(Pair.Key, Pair.Value.CommandClass);
        }
        else
        {
            RuntimeLocalCommands.Add(Pair.Key, Pair.Value.CommandClass);
        }
    }

    // 4. Переопределяем прослойку устройства, если задана
    if (OverrideDeviceProxyCommand)
    {
        RuntimeDeviceProxyCommand = OverrideDeviceProxyCommand;
    }
}

// Вспомогательная функция поиска наибольшего общего префикса среди подходящих слов
static FString GetLongestCommonPrefix(const TArray<FString>& Strings)
{
    if (Strings.IsEmpty()) return FString();
    FString Prefix = Strings[0];

    for (int32 i = 1; i < Strings.Num(); ++i)
    {
        int32 j = 0;
        while (j < Prefix.Len() && j < Strings[i].Len() &&
            FChar::ToLower(Prefix[j]) == FChar::ToLower(Strings[i][j]))
        {
            j++;
        }
        Prefix.LeftInline(j);
    }
    return Prefix;
}

FAutocompleteResult UTerminalComponent::ProcessAutocomplete(const FString& RawInput, const FTerminalContext& Context)
{
    FAutocompleteResult Result;
    Result.CompletedInput = RawInput;

    FString LeftTrimmed = RawInput.TrimStart();
    bool bEndsWithSpace = LeftTrimmed.EndsWith(TEXT(" "));

    TArray<FString> Tokens;
    LeftTrimmed.ParseIntoArray(Tokens, TEXT(" "), true);

    // СЦЕНАРИЙ 1: Игрок вводит первое слово (команду)
    if (Tokens.Num() == 0 || (Tokens.Num() == 1 && !bEndsWithSpace))
    {
        FString Prefix = (Tokens.Num() == 1) ? Tokens[0] : TEXT("");

        // ВСЕГДА добавляем подходящие глобальные команды терминала (help, exit, disconnect, clear)
        for (const TPair<FName, TSubclassOf<UTerminalCommand>>& Pair : RuntimeGlobalCommands)
        {
            FString CmdStr = Pair.Key.ToString();
            if (CmdStr.StartsWith(Prefix, ESearchCase::IgnoreCase))
            {
                Result.Suggestions.AddUnique(CmdStr.ToLower());
            }
        }

        // Если терминал подключен к устройству — добираем команды устройства через прослойку
        if (ConnectedDevice != NAME_None)
        {
            if (RuntimeDeviceProxyCommand)
            {
                const UTerminalCommand* ProxyCDO = RuntimeDeviceProxyCommand->GetDefaultObject<UTerminalCommand>();

                // Получаем команды устройства в отдельный буфер, чтобы не затереть глобальные
                TArray<FString> DeviceCmds = ProxyCDO->GetSuggestions(
                    NAME_None, true, 0, Prefix, Context);

                for (const FString& DevCmd : DeviceCmds)
                {
                    // Дополнительная проверка префикса и защита от дубликатов с Global
                    if (DevCmd.StartsWith(Prefix, ESearchCase::IgnoreCase))
                    {
                        Result.Suggestions.AddUnique(DevCmd.ToLower());
                    }
                }
            }
        }
        // Если не подключены к устройству — добираем локальные команды терминала (connect, list)
        else
        {
            for (const TPair<FName, TSubclassOf<UTerminalCommand>>& Pair : RuntimeLocalCommands)
            {
                FString CmdStr = Pair.Key.ToString();
                if (CmdStr.StartsWith(Prefix, ESearchCase::IgnoreCase))
                {
                    Result.Suggestions.AddUnique(CmdStr.ToLower());
                }
            }
        }

        // Сортируем объединенный список (Global + Device/Local) по алфавиту для аккуратного вывода в UI
        Result.Suggestions.Sort();

        // Формируем итоговую строку автодополнения
        if (Result.Suggestions.Num() == 1)
        {
            Result.CompletedInput = Result.Suggestions[0] + TEXT(" ");
        }
        else if (Result.Suggestions.Num() > 1)
        {
            Result.CompletedInput = GetLongestCommonPrefix(Result.Suggestions);
        }

        return Result;
    }

    // СЦЕНАРИЙ 2: Игрок вводит аргумент после пробела
    FString CommandKeyStr = Tokens[0];
    FName CommandName(*CommandKeyStr, FNAME_Find);
    if (CommandName.IsNone()) return Result;

    int32 CurrentArgIndex = bEndsWithSpace ? (Tokens.Num() - 1) : (Tokens.Num() - 2);
    FString ArgPrefix = bEndsWithSpace ? TEXT("") : Tokens.Last();

    const UTerminalCommand* TargetCommandCDO = nullptr;

    // Приоритет поиска владельца аргументов строго совпадает с ProcessInput:
    // Сначала проверяем Global, затем DeviceProxy (если подключены), затем Local
    if (TSubclassOf<UTerminalCommand>* GlobalPtr = RuntimeGlobalCommands.Find(CommandName))
    {
        TargetCommandCDO = (*GlobalPtr)->GetDefaultObject<UTerminalCommand>();
    }
    else if (ConnectedDevice != NAME_None && RuntimeDeviceProxyCommand)
    {
        TargetCommandCDO = RuntimeDeviceProxyCommand->GetDefaultObject<UTerminalCommand>();
    }
    else if (TSubclassOf<UTerminalCommand>* LocalPtr = RuntimeLocalCommands.Find(CommandName))
    {
        TargetCommandCDO = (*LocalPtr)->GetDefaultObject<UTerminalCommand>();
    }

    if (TargetCommandCDO)
    {
        TArray<FString> RawArgSuggestions = TargetCommandCDO->GetSuggestions(
            CommandName, false, CurrentArgIndex, ArgPrefix, Context);

        for (const FString& Suggestion : RawArgSuggestions)
        {
            if (Suggestion.StartsWith(ArgPrefix, ESearchCase::IgnoreCase))
            {
                Result.Suggestions.AddUnique(Suggestion);
            }
        }
        Result.Suggestions.Sort();
    }

    if (!Result.Suggestions.IsEmpty())
    {
        FString CompletedArg = (Result.Suggestions.Num() == 1)
            ? (Result.Suggestions[0] + TEXT(" "))
            : GetLongestCommonPrefix(Result.Suggestions);

        FString BaseString = bEndsWithSpace ? LeftTrimmed : LeftTrimmed.LeftChop(ArgPrefix.Len());
        Result.CompletedInput = BaseString + CompletedArg;
    }

    return Result;
}

TMap<FName, TSubclassOf<UTerminalCommand>> UTerminalComponent::GetCommandRegistry_Implementation() const
{
    TMap<FName, TSubclassOf<UTerminalCommand>> Result = RuntimeGlobalCommands;
    Result.Reserve(Result.Num() + RuntimeLocalCommands.Num());
    Result.Append(RuntimeLocalCommands);
    return Result;
}

void UTerminalComponent::ClearTerminalScreen_Implementation()
{

}

void UTerminalComponent::CloseTerminal_Implementation()
{

}

void UTerminalComponent::ConnectToDevice_Implementation(FName DeviceID)
{
    ConnectedDevice = DeviceID;
}

void UTerminalComponent::DisconnectFromDevice_Implementation()
{
    ConnectedDevice = NAME_None;
}

FName UTerminalComponent::GetConnectedDevice_Implementation()
{
    return ConnectedDevice;
}
