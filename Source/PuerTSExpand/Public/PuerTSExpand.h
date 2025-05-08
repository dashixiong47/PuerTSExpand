// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PuerTSExpandStruct.h"
#include "Modules/ModuleManager.h"

class FPuerTSExpandModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
protected:
	FDelegateHandle ContentBrowserExtenderDelegateHandle;
	void AddContentBrowserContextMenuExtender();
	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	static void GenerateMixinTemplate(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);

	static FGenerateStatus GenerateMixinFileFromTemplate(const TMap<FString, FString>& Replacements,const FString& FilePath,const FString& FileName);
	static FGenerateStatus DeleteMixinFileFromAsset(const FAssetData& Asset);
	static void AddImportStatementIfNotExists(const FString& RelativeImportPath);
	static void RemoveImportStatementIfExists(const FString& RelativeImportPath);
};
