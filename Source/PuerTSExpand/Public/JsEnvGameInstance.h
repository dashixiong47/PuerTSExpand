// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsEnv.h"
#include "Engine/GameInstance.h"
#include "JsEnvGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PUERTSEXPAND_API UJsEnvGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "JsEnv")
	FString ModuleName = TEXT("MainGame");

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "JsEnv")
	bool bDebugMode = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "JsEnv")
	bool bWaitForDebugger = false;
	
	TSharedPtr<puerts::FJsEnv> JsEnv;

	virtual void OnStart() override;
	virtual void Shutdown() override;
};
