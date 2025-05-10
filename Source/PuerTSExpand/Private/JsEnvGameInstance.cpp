// Fill out your copyright notice in the Description page of Project Settings.


#include "JsEnvGameInstance.h"

void UJsEnvGameInstance::OnStart()
{
	Super::OnStart();
	if (bDebugMode)
	{
		JsEnv = MakeShared<puerts::FJsEnv>(std::make_unique<puerts::DefaultJSModuleLoader>(TEXT("JavaScript")),
		                                   std::make_shared<puerts::FDefaultLogger>(), 8080);
		if (bWaitForDebugger)
		{
			JsEnv->WaitDebugger();
		}
		
	}
	else
	{
		JsEnv = MakeShared<puerts::FJsEnv>();
	}
	TArray<TPair<FString, UObject*>> Arguments;
	Arguments.Add(TPair<FString, UObject*>(TEXT("GameInstance"), this));
	JsEnv->Start(ModuleName, Arguments);
}

void UJsEnvGameInstance::Shutdown()
{
	Super::Shutdown();
	JsEnv.Reset();
}
