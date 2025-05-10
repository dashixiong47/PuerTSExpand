// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuerTSExpand.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "IDeclarationGenerator.h"
#include "ISettingsModule.h"
#include "PuertsExpandSettings.h"
#include "PuerTSExpandStruct.h"
#include "Editor/UMGEditor/Public/WidgetBlueprint.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FPuerTSExpandModule"

void FPuerTSExpandModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "PuertsExpand",
		                                 NSLOCTEXT("PuertsExpand", "PuertsExpandSettingsName", "Puerts Expand"),
		                                 NSLOCTEXT("PuertsExpand", "PuertsExpandSettingsDescription",
		                                           "Settings for Puerts Expand Plugin."),
		                                 GetMutableDefault<UPuertsExpandSettings>());
	}
	
	AddContentBrowserContextMenuExtender();
	StartNpmDev();
	
	
	// FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	// AssetToolsModule.Get().OnAssetPostRename().AddRaw(this, &FPuerTSExpandModule::OnAssetPostRename);
}

void FPuerTSExpandModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().OnAssetPostRename().RemoveAll(this);
	}
}

void FPuerTSExpandModule::AddContentBrowserContextMenuExtender()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.
		GetAllAssetViewContextMenuExtenders();

	CBMenuAssetExtenderDelegates.Add(
		FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&OnExtendContentBrowserAssetSelectionMenu));
	ContentBrowserExtenderDelegateHandle = CBMenuAssetExtenderDelegates.Last().GetHandle();
}

TSharedRef<FExtender> FPuerTSExpandModule::OnExtendContentBrowserAssetSelectionMenu(
	const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension(
		"CommonAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(&AddExpandButton, SelectedAssets)
	);
	return Extender;
}

void FPuerTSExpandModule::AddExpandButton(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
	// 先调用PuerTS生成TS类
	IDeclarationGenerator& Generator = IDeclarationGenerator::Get();
	Generator.GenTypeScriptDeclaration(false, NAME_None);
	
	bool bIShow = true;
	for (const FAssetData& Asset : SelectedAssets)
	{
		const FTopLevelAssetPath& ClassPath = Asset.AssetClassPath;
		if (ClassPath != UBlueprint::StaticClass()->GetClassPathName() && ClassPath != UWidgetBlueprint::StaticClass()->
			GetClassPathName())
		{
			bIShow = false;
			break;
		}
	}
	if (!bIShow)
	{
		return;
	}
	MenuBuilder.BeginSection("PuerTSExpand", LOCTEXT("PuerTSExpand", "PuerTS扩展"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("GenerateMixinTemplateLabel", "生成Mixin模板"),
			LOCTEXT("GenerateMixinTemplateTooltip", "生成Mixin模板"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				FString SuccessTips = TEXT("生成Mixin模板成功:\n");
				FString FailedTips = TEXT("生成Mixin模板失败:\n");
				FString WarningTips = TEXT("警告:\n");
				for (const FAssetData& Asset : SelectedAssets)
				{
					// 资源名（不含路径）
					FString AssetName = Asset.AssetName.ToString(); // 例如 "MyCharacterBP"

					// 虚拟路径（不含资源名）
					FString AssetPath = Asset.PackagePath.ToString(); // 例如 "/Game/MyCharacterBP"

					// 完整虚拟路径（含资源名，不含扩展名）
					FString FullObjectPath = Asset.GetObjectPathString(); // 例如 "/Game/Characters/MyCharacterBP.MyCharacterBP"

					TMap<FString, FString> Replacements;
					Replacements.Add(TEXT("<AssetName>"), AssetName);
					Replacements.Add(TEXT("<AssetPath>"), AssetPath);
					Replacements.Add(TEXT("<FullObjectPath>"), FullObjectPath);
					FString ProcessedPath = SanitizeNumericPathSegments(FullObjectPath);
					FString AssetTypes = ProcessedPath.Replace(TEXT("/"), TEXT("."));
					Replacements.Add(TEXT("<AssetTypes>"), FString::Printf(TEXT("UE%s_C"), *AssetTypes));
					FString NewFilePath = AssetPath.Replace(TEXT("/Game"), TEXT(""));
					FGenerateStatus CallbackStatus = FPuerTSExpandModule::GenerateMixinFileFromTemplate(
						Replacements, NewFilePath,TEXT("/") + AssetName);
					if (CallbackStatus.Status == EGenerateStatus::Success)
					{
						SuccessTips += CallbackStatus.Message + TEXT("\n");
					}
					else if (CallbackStatus.Status == EGenerateStatus::Failed)
					{
						FailedTips += CallbackStatus.Message + TEXT("\n");
					}
					else if (CallbackStatus.Status == EGenerateStatus::Warning)
					{
						WarningTips += CallbackStatus.Message + TEXT("\n");
					}
				}
				FString Message = FString::Printf(TEXT("%s\n\n%s\n\n%s\n\n"), *SuccessTips, *FailedTips, *WarningTips);
				// 使用新的重载方法，传递标题作为值
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), FText::FromString(TEXT("提示")));
			})),
			NAME_None,
			EUserInterfaceActionType::Button
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("GenerateMixinTemplateLabel", "清除Mixin模板"),
			LOCTEXT("GenerateMixinTemplateTooltip", "清除Mixin模板"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				FText DialogTitle = FText::FromString(TEXT("确认删除"));
				FText DialogMessage = FText::FromString(TEXT("确定要删除这些Mixin文件吗？"));
				EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogMessage, DialogTitle);

				if (Response == EAppReturnType::Yes)
				{
					FString SuccessTips = TEXT("删除成功:\n");
					FString FailedTips = TEXT("删除失败:\n");
					FString WarningTips = TEXT("警告:\n");

					for (const FAssetData& Asset : SelectedAssets)
					{
						FGenerateStatus Status = FPuerTSExpandModule::DeleteMixinFileFromAsset(Asset);
						if (Status.Status == EGenerateStatus::Success)
						{
							SuccessTips += Status.Message + TEXT("\n");
						}
						else if (Status.Status == EGenerateStatus::Failed)
						{
							FailedTips += Status.Message + TEXT("\n");
						}
						else if (Status.Status == EGenerateStatus::Warning)
						{
							WarningTips += Status.Message + TEXT("\n");
						}
					}

					FString Message = FString::Printf(TEXT("%s\n%s\n%s"), *SuccessTips, *FailedTips, *WarningTips);
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), DialogTitle);
				}
				else
				{
					FText CancelMessage = FText::FromString(TEXT("删除操作已取消"));
					FMessageDialog::Open(EAppMsgType::Ok, CancelMessage, DialogTitle);
				}
			})), NAME_None,
			EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();
}

FGenerateStatus FPuerTSExpandModule::GenerateMixinFileFromTemplate(const TMap<FString, FString>& Replacements,
                                                                   const FString& FilePath, const FString& FileName)
{
	FGenerateStatus Status;
	// 获取输出目录并确保路径格式标准
	FString Path = GetDefault<UPuertsExpandSettings>()->OutputPath;
	FString OutputDir = FPaths::Combine(FPaths::ProjectDir(), Path, FilePath);
	IFileManager::Get().MakeDirectory(*OutputDir, true); // 确保目录存在
	FString OutputPath = FPaths::Combine(OutputDir, FileName + TEXT(".ts"));

	// 检查文件是否存在
	if (IFileManager::Get().FileExists(*OutputPath))
	{
		AddImportStatementIfNotExists(FilePath + FileName);
		Status.Status = EGenerateStatus::Warning;
		Status.Message = FString::Printf(TEXT("文件已存在，无法覆盖: %s"), *OutputPath);
		return Status;
	}

	// 获取插件路径
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("PuerTSExpand"))->GetBaseDir();
	FString TemplatePath = FPaths::Combine(PluginDir, TEXT("Source/PuerTSExpand/Template/Mixin.ts"));

	// 读取模板文件内容
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *TemplatePath))
	{
		Status.Status = EGenerateStatus::Failed;
		Status.Message = FString::Printf(TEXT("读取模板文件失败: %s"), *TemplatePath);
		return Status;
	}

	// 遍历替换表，依次替换所有占位符
	for (const TPair<FString, FString>& Pair : Replacements)
	{
		const FString& Placeholder = Pair.Key;
		const FString& Replacement = Pair.Value;
		FileContent.ReplaceInline(*Placeholder, *Replacement, ESearchCase::IgnoreCase);
	}

	// 保存生成的 Mixin 文件
	if (FFileHelper::SaveStringToFile(FileContent, *OutputPath))
	{
		// 自动导入文件路径
		AddImportStatementIfNotExists(FilePath + FileName);
		
		Status.Status = EGenerateStatus::Success;
		Status.Message = FString::Printf(TEXT("Mixin 文件已生成: %s"), *OutputPath);
		return Status;
	}
	else
	{
		Status.Status = EGenerateStatus::Failed;
		Status.Message = FString::Printf(TEXT("写入文件失败: %s"), *OutputPath);
		return Status;
	}
}
FGenerateStatus FPuerTSExpandModule::DeleteMixinFileFromAsset(const FAssetData& Asset)
{
	FGenerateStatus Status;

	FString AssetName = Asset.AssetName.ToString();
	FString AssetPath = Asset.PackagePath.ToString().Replace(TEXT("/Game"), TEXT(""));
	FString FileName = TEXT("/") + AssetName;

	// 构造 Mixin 文件路径
	FString Path = GetDefault<UPuertsExpandSettings>()->OutputPath;
	
	FString OutputDir = FPaths::Combine(FPaths::ProjectDir(), Path, AssetPath);
	FString OutputJSDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JavaScript"), AssetPath);
	
	FString OutputPath = FPaths::Combine(OutputDir, FileName + TEXT(".ts"));
	FString OutputJSPath = FPaths::Combine(OutputJSDir, FileName + TEXT(".js"));

	// 构造 import 文件路径
	FString AutoImportFileName = GetDefault<UPuertsExpandSettings>()->AutoImportFileName;
	FString AutoImportPath = FPaths::Combine(FPaths::ProjectDir(), Path, AutoImportFileName);

	// 准备删除结果提示
	bool bDeleteSuccess = true;
	FString Tips;

	// 删除 Mixin 文件
	if (IFileManager::Get().FileExists(*OutputPath))
	{
		// 删除TS文件和JS文件和map文件
		IFileManager::Get().Delete(*(OutputJSPath+TEXT(".map")));
		IFileManager::Get().Delete(*OutputJSPath);
	
		if (!IFileManager::Get().Delete(*OutputPath))
		{
			Status.Status = EGenerateStatus::Failed;
			Status.Message = FString::Printf(TEXT("删除失败: %s"), *OutputPath);
			return Status;
		}
		
	}
	else
	{
		RemoveImportStatementIfExists(AssetPath + FileName);
		Tips += FString::Printf(TEXT("文件不存在: %s\n"), *OutputPath);
	}

	// 删除对应 import 语句
	if (FPaths::FileExists(AutoImportPath))
	{
		RemoveImportStatementIfExists(AssetPath + FileName);
	}

	// 返回最终状态
	if (bDeleteSuccess)
	{
		Status.Status = EGenerateStatus::Success;
		Status.Message = FString::Printf(TEXT("已删除文件及import引用: %s"), *OutputPath);
	}
	else
	{
		Status.Status = EGenerateStatus::Warning;
		Status.Message = FString::Printf(TEXT("文件删除成功，但 import 删除存在问题: %s\n%s"), *OutputPath, *Tips);
	}
	return Status;
}

void FPuerTSExpandModule::AddImportStatementIfNotExists(const FString& RelativeImportPath)
{
	FString Path = GetDefault<UPuertsExpandSettings>()->OutputPath;
	FString AutoImportFileName = GetDefault<UPuertsExpandSettings>()->AutoImportFileName;
	FString FullFilePath = FPaths::Combine(FPaths::ProjectDir(), Path, AutoImportFileName);

	// 读取内容
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FullFilePath))
	{
		FString ImportLine = FString::Printf(TEXT("import '.%s';"), *RelativeImportPath);

		if (!FileContent.Contains(ImportLine))
		{
			FileContent += FString::Printf(TEXT("\n%s"), *ImportLine);
			FFileHelper::SaveStringToFile(FileContent, *FullFilePath);
		}
	}else
	{
		// 如果文件不存在，则创建一个新的文件并写入 import 语句
		FString NewContent = FString::Printf(TEXT("import '.%s';"), *RelativeImportPath);
		FFileHelper::SaveStringToFile(NewContent, *FullFilePath);
	}
}
void FPuerTSExpandModule::RemoveImportStatementIfExists(const FString& RelativeImportPath)
{
	FString Path = GetDefault<UPuertsExpandSettings>()->OutputPath;
	FString AutoImportFileName = GetDefault<UPuertsExpandSettings>()->AutoImportFileName;
	FString FullFilePath = FPaths::Combine(FPaths::ProjectDir(), Path, AutoImportFileName);

	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FullFilePath))
	{
		FString ImportLine = FString::Printf(TEXT("import '.%s';"), *RelativeImportPath);
		
		// 尝试逐行过滤掉目标 import 行
		TArray<FString> Lines;
		FileContent.ParseIntoArrayLines(Lines);
		Lines.RemoveAll([&](const FString& Line) {
			return Line.TrimStartAndEnd().Equals(ImportLine);
		});

		// 重组并写入
		FString NewContent = FString::Join(Lines, TEXT("\n"));
		FFileHelper::SaveStringToFile(NewContent, *FullFilePath);
	}
}

FString FPuerTSExpandModule::SanitizeNumericPathSegments(const FString& Path)
{
	auto IsNumericOnly = [](const FString& Str) -> bool
	{
		for (TCHAR Char : Str)
		{
			if (!FChar::IsDigit(Char))
			{
				return false;
			}
		}
		return Str.Len() > 0;
	};

	// 拆分路径
	TArray<FString> PathSegments;
	Path.ParseIntoArray(PathSegments, TEXT("/"), true);

	// 处理纯数字段
	for (FString& Segment : PathSegments)
	{
		if (IsNumericOnly(Segment))
		{
			Segment = TEXT("_") + Segment;
		}
	}

	// 重新组合路径（加上开头的 "/"）
	return TEXT("/") + FString::Join(PathSegments, TEXT("/"));
}

void FPuerTSExpandModule::OnAssetPostRename(const TArray<FAssetRenameData>& RenameDataList)
{
	// 遍历重命名数据列表
	for (const FAssetRenameData& RenameData : RenameDataList)
	{
		// 获取旧路径
		FString OldPath = RenameData.OldObjectPath.GetAssetName();

		// 获取旧的文件名（没有扩展名的名称）
		FString OldName = RenameData.OldObjectPath.ToString(); //获取不到 先不做重命名更新

		// 获取新路径和新名称
		FString NewPath = RenameData.Asset->GetPathName();
		FString NewName = RenameData.NewName;

		UE_LOG(LogTemp, Log, TEXT("资源重命名："));
		UE_LOG(LogTemp, Log, TEXT("旧路径：%s"), *OldPath);
		UE_LOG(LogTemp, Log, TEXT("旧资源名称：%s"), *OldName);
		UE_LOG(LogTemp, Log, TEXT("新路径：%s"), *NewPath);
		UE_LOG(LogTemp, Log, TEXT("新资源名称：%s"), *NewName);

	}
}

void FPuerTSExpandModule::StartNpmDev()
{
	FString NpmDevCommand=GetDefault<UPuertsExpandSettings>()->NpmDevCommand;
	if(NpmDevCommand==TEXT(""))
	{
		return;
	}
	const FString Command = TEXT("cmd.exe");
	const FString Params = NpmDevCommand;
	const FString WorkingDir = FPaths::ProjectDir();

	NpmDevProcessHandle = FPlatformProcess::CreateProc(
		*Command,
		*Params,
		true,   // Detached
		false,  // Hidden
		false,  // ReallyHidden
		nullptr,
		0,
		*WorkingDir,
		nullptr
	);

	if (NpmDevProcessHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("npm run dev 启动成功"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("启动失败"));
	}
}

void FPuerTSExpandModule::StopNpmDev()
{
	if (NpmDevProcessHandle.IsValid())
	{
		FPlatformProcess::TerminateProc(NpmDevProcessHandle, true);
		NpmDevProcessHandle.Reset();
		UE_LOG(LogTemp, Log, TEXT("npm run dev 已停止"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("没有正在运行的 npm 进程"));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPuerTSExpandModule, PuerTSExpand)
