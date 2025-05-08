// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PuertsExpandSettings.generated.h"

/**
 * 
 */
UCLASS(Config=PuertsExpand)
class PUERTSEXPAND_API UPuertsExpandSettings : public UObject
{
	GENERATED_BODY()
public:

	/** 输出路径 */
	UPROPERTY(EditAnywhere, Config, Category="PuertsExpand")
	FString OutputPath = TEXT("/TypeScript");

	/** 自动Import 文件名称 */
	UPROPERTY(EditAnywhere, Config, Category="PuertsExpand")
	FString AutoImportFileName = TEXT("MainGame.ts");
};
