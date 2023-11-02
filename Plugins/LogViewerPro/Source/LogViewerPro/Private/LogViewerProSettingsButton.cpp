// Copyright Dmitrii Labadin 2019

#include "LogViewerProSettingsButton.h"
#include "LogViewerProWidgetCategoriesView.h"
#include "LogViewerProWidgetMain.h"
#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"

#include "Misc/FileHelper.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"

#include "LogViewerProStructs.h"

#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerProSettingsButton::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;

	SComboButton::Construct(SComboButton::FArguments()
		.ContentPadding(0)
		.ForegroundColor(FSlateColor::UseForeground())
		.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButton")
		.HasDownArrow(false)
		.OnGetMenuContent(this, &SLogViewerProSettingsButton::GenerateSettingsMenu)
		.ButtonContent()
	[
		SNew(SHorizontalBox)
		
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(FMargin(4.0, 2.0))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Settings", "Settings"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(FMargin(4.0, 2.0))
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.Settings"))
				.ColorAndOpacity(FSlateColor::UseForeground())
		]
	]
	);

	//FSlateColorBrush brushClr(FLinearColor::White);
}

//SETTINGS

TSharedRef<SWidget> SLogViewerProSettingsButton::GenerateSettingsMenu()
{
	// Get all menu extenders for this context menu from the content browser module

	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, nullptr, nullptr, /*bCloseSelfOnly=*/ true);

	MenuBuilder.AddMenuEntry(LOCTEXT("LogViewerSettingsVisualSettings_SaveSettings", "Save Settings"), LOCTEXT("LogViewerSettingsVisualSettings_SaveSettingsTooltip", "Make current settings configuration default"), FSlateIcon(), FUIAction(FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::SaveSettings)), NAME_None, EUserInterfaceActionType::Button);
	//MenuBuilder.AddMenuEntry(LOCTEXT("CollapseAll", "Collapse All"), LOCTEXT("CollapseAll_Tooltip", "Collapses the entire tree"), FSlateIcon(), FUIAction(FExecuteAction::CreateSP(this, &SClassViewer::SetAllExpansionStates, bool(false))), NAME_None, EUserInterfaceActionType::Button);

	MenuBuilder.BeginSection("Behavior", LOCTEXT("LVSettingsBehavior", "Behavior"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LVPSettingsCleanUp", "Clean Up On PIE"),
			LOCTEXT("LVPSettingsCleanUpTooltip", "Clean up logs each time PIE starts"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleCleanUpLogsOnPie),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsCleanUpLogsOnPie)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("Visual", LOCTEXT("LogViewerSettingsVisualSettings", "Customize Columns"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowTimeColumn", "Show Time Column"),
			LOCTEXT("LogViewerSettingsVisualShowTimeColumnToolTip", "Show/Hide time column if you need a more compat view"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleShowTime),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsShowTime)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowTimeColumn", "Show Seconds Column"),
			LOCTEXT("LogViewerSettingsVisualShowTimeColumnToolTip", "Show/Hide seconds column if you need a more compat view"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleShowSeconds),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsShowSeconds)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowTimeColumn", "Show Frame Column"),
			LOCTEXT("LogViewerSettingsVisualShowTimeColumnToolTip", "Show/Hide Frame column if you need a more compat view"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleShowFrame),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsShowFrame)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualCompactRow", "Compact rows"),
			LOCTEXT("LogViewerSettingsVisualCompactRowToolTip", "If row contains multiline log message, it will be collapsed to one line for a compact view. (You can view full message, by clicking on it)"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleCompactRow),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsCompactRow)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("Visual", LOCTEXT("LogViewerSettingsVisualSettingsMinimap", "Minimap"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowMinimap", "Show Minimap"),
			LOCTEXT("LogViewerSettingsVisualShowMinimapToolTip", "Hide minimap to reclaim more space"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleShowMinimap),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsShowMinimap)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();


	MenuBuilder.BeginSection("LogCategories", LOCTEXT("LogViewerSettingsLogCategories", "Colorcoding"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowLogColors", "Colorful logs"),
			LOCTEXT("LogViewerSettingsVisualShowLogColorsToolTip", "Switch between legacy view and colored view"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleShowLogColors),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsShowLogColors)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualShowLogColors", "Colorful minimap"),
			LOCTEXT("LogViewerSettingsVisualShowLogColorsToolTip", "Apply colorcoding to minimap for better navigation."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::ToggleColorfulMinimap),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SLogViewerProSettingsButton::IsShowColorfulMinimap)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("LogSupport", LOCTEXT("LogViewerSettingsSupport", "Support"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("LogViewerSettingsVisualSettings_JoinDis", "Join Discord"),
			LOCTEXT("LogViewerSettingsVisualSettings_JoinDisTooltip", "If you have questions about the plugin you are welcome in official Discord."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SLogViewerProSettingsButton::JoinDiscordServer)), NAME_None, EUserInterfaceActionType::Button);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SLogViewerProSettingsButton::ToggleShowMinimap()
{
	SettingsData.bShowMinimap = !SettingsData.bShowMinimap;
	MainWidget->UpdateShowMinimap();
}

bool SLogViewerProSettingsButton::IsShowMinimap() const
{
	return SettingsData.bShowMinimap;
}


void SLogViewerProSettingsButton::ToggleShowTime()
{
	SettingsData.bShowTime = !SettingsData.bShowTime;
	MainWidget->UpdateShowTime();
}

bool SLogViewerProSettingsButton::IsShowTime() const
{
	return SettingsData.bShowTime;
}

void SLogViewerProSettingsButton::ToggleShowSeconds()
{
	SettingsData.bShowSeconds = !SettingsData.bShowSeconds;
	MainWidget->UpdateShowTime();
}

bool SLogViewerProSettingsButton::IsShowSeconds() const
{
	return SettingsData.bShowSeconds;
}

bool SLogViewerProSettingsButton::IsShowFrame() const
{
	return SettingsData.bShowFrame;
}

void SLogViewerProSettingsButton::ToggleShowFrame()
{
	SettingsData.bShowFrame = !SettingsData.bShowFrame;
	MainWidget->UpdateShowTime();
}

void SLogViewerProSettingsButton::ToggleCompactRow()
{
	SettingsData.bCompactRow = !SettingsData.bCompactRow;
	MainWidget->Refresh();

}

bool SLogViewerProSettingsButton::IsShowFrameNum() const
{
	return SettingsData.bShowFrame;
}

bool SLogViewerProSettingsButton::IsCompactRow() const
{
	return SettingsData.bCompactRow;
}

void SLogViewerProSettingsButton::ToggleShowLogColors()
{
	SettingsData.bColorfulLogs = !SettingsData.bColorfulLogs;
	MainWidget->Refresh();
}

bool SLogViewerProSettingsButton::IsShowLogColors() const
{
	return SettingsData.bColorfulLogs;
}

void SLogViewerProSettingsButton::ToggleColorfulMinimap()
{
	SettingsData.bColorfulMinimap = !SettingsData.bColorfulMinimap;
	MainWidget->UpdateShowMinimap();
}



bool SLogViewerProSettingsButton::IsShowColorfulMinimap() const
{
	return SettingsData.bColorfulMinimap;
}

bool SLogViewerProSettingsButton::IsCleanUpLogsOnPie() const
{
	return SettingsData.bCleanUpLogsOnPIE;
}

void SLogViewerProSettingsButton::ToggleCleanUpLogsOnPie()
{
	SettingsData.bCleanUpLogsOnPIE = !SettingsData.bCleanUpLogsOnPIE;
}

void SLogViewerProSettingsButton::TryLoadAndApplySettings()
{
	FString Path = GetDefaultSettingsFilePath();
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString , *Path))
	{
		return;
	}

	SettingsData.FromJson(JsonString);

	MainWidget->UpdateShowTime();
	MainWidget->UpdateShowMinimap();
	MainWidget->Refresh();
}

void SLogViewerProSettingsButton::SaveSettings()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "Saving is not supported on this platform!"));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString DefaultPath = GetDefaultSettingsFolderPath();
	FPaths::NormalizeDirectoryName(DefaultPath);
	if (!PlatformFile.DirectoryExists(*DefaultPath))
	{
		PlatformFile.CreateDirectory(*DefaultPath);
	}

	FString Filename = GetDefaultSettingsFilePath();

	// save file
	FArchive* LogFile = IFileManager::Get().CreateFileWriter(*Filename);

	if (LogFile != nullptr)
	{

		const FString SettingsString = SettingsData.ToJson(true);
		LogFile->Serialize(TCHAR_TO_ANSI(*SettingsString), SettingsString.Len());
		LogFile->Close();
		delete LogFile;
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogFileError", "Failed to save settings to save file. Please that file: Project/Config/LogViewer/LogViewerSettings.ini is writeable."));
	}
}

void SLogViewerProSettingsButton::JoinDiscordServer()
{
	const FString Link = TEXT("https://discord.gg/aYFGHpb2pa");
	FPlatformProcess::LaunchURL(*Link, nullptr, nullptr);
}

FString SLogViewerProSettingsButton::GetDefaultSettingsFolderPath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewer");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}


//@TODO move to helper function and replace all entries
FString SLogViewerProSettingsButton::GetDefaultSettingsFilePath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewerPro") + TEXT("/LogViewerSettings.ini");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE