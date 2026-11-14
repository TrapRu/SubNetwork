// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/SubNetworkUtilsLibrary.h"

UObject* USubNetworkUtilsLibrary::GetClassDefaultObject(TSubclassOf<UObject> ObjectClass)
{
    if (ObjectClass)
    {
        return ObjectClass->GetDefaultObject();
    }

    return nullptr;
}