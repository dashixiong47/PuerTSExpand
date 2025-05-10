// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAssetTools.h"
#include "PuerTSExpandStruct.h"
#include "Modules/ModuleManager.h"

class FPuerTSExpandModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	FProcHandle NpmDevProcessHandle;
protected:
	FDelegateHandle ContentBrowserExtenderDelegateHandle;
	void AddContentBrowserContextMenuExtender();
	
	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	
	static void AddExpandButton(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);

	// 生成Mixin文件 返回状态
	static FGenerateStatus GenerateMixinFileFromTemplate(const TMap<FString, FString>& Replacements,const FString& FilePath,const FString& FileName);

	// 删除Mixin文件 返回状态
	static FGenerateStatus DeleteMixinFileFromAsset(const FAssetData& Asset);
	
	// 添加导入语句
	static void AddImportStatementIfNotExists(const FString& RelativeImportPath);
	
	// 删除导入语句
	static void RemoveImportStatementIfExists(const FString& RelativeImportPath);
	
	// 将路径中纯数字的段加前缀 "_"
	static  FString SanitizeNumericPathSegments(const FString& Path);

	// 监听蓝图重命名
	void OnAssetPostRename(const TArray<FAssetRenameData>& RenameDataList);

	// 命令行运行Node
	
	void StartNodeCommand();
	void StopNodeCommand();
};
