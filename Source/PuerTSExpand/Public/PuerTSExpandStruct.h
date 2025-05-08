// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PuerTSExpandStruct.generated.h"


UENUM(BlueprintType)
enum class EGenerateStatus : uint8
{
	Success UMETA(DisplayName = "Success", ToolTip = "成功"),
	Failed  UMETA(DisplayName = "Failed", ToolTip = "失败"),
	Warning UMETA(DisplayName = "Warning", ToolTip = "警告"),
};

USTRUCT(BlueprintType)
struct FGenerateStatus
{
	GENERATED_BODY()
	EGenerateStatus Status = EGenerateStatus::Success;
	FString Message;
};
