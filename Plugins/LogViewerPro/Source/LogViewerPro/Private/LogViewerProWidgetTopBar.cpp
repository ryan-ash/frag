// Copyright Dmitrii Labadin 2019

#include "LogViewerProWidgetTopBar.h"
#include "LogViewerProWidgetCategoriesView.h"
#include "LogViewerProWidgetMain.h"
#include "LogViewerProSettingsButton.h"
#include "SlateOptMacros.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "ProjectDescriptor.h"
#include "EditorDirectories.h"
#include "Settings/EditorSettings.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/DOcking/SDockTab.h"
#include "Widgets/Input/SSearchBox.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "LogViewerProStructs.h"

#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerProWidgetTopBar::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;
	CategoryMenu = InArgs._CategoryMenu;
	
	//FSlateColorBrush brushClr(FLinearColor::White);

	SettingsButton = SNew(SLogViewerProSettingsButton).MainWidget(MainWidget); ;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(0)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnOpenFilePressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Open Log", "Open Log"))
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnSaveFilePressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Save Log", "Save Log"))
					]
				]
			]
			
			//+ SHorizontalBox::Slot()
			//.AutoWidth()
			//.HAlign(HAlign_Right)
			//.Padding(FMargin(5.0f, 0.0f))
			//[
			//	SNew(SCheckBox)
			//	.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
			//	.IsChecked_Lambda([&]() { return MainWidget->IsShowTime() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			//	.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnShowTimePressed)
			//	[
			//		SNew(SBox)
			//		.VAlign(VAlign_Center)
			//		.HAlign(HAlign_Center)
			//		.Padding(FMargin(4.0, 2.0))
			//		[
			//			SNew(STextBlock)
			//			.Text(LOCTEXT("LogViewerProShowTime", "Time"))
			//		]
			//	]
			//]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnClearOutputPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SAssignNew(ClearOutputCaption, STextBlock)
						.Text_Lambda([&]() { return MainWidget->LogMessages.Num() > 0 ? LOCTEXT("LogViewerProClearOutput", "Clear Output") : LOCTEXT("LogViewerProClearOutput", "Clear Categories"); })
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.35)
			.Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
			[
				SAssignNew(FilterTextBox, SSearchBox)
				.HintText(LOCTEXT("FilterLogHint", "Filter Log (Alt + F)"))
				.OnTextCommitted(this, &SLogViewerProWidgetTopBar::OnFilterTextCommitted)
				.OnTextChanged(this, &SLogViewerProWidgetTopBar::OnFilterTextChanged)
				.DelayChangeNotificationsWhileTyping(true)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.FillWidth(0.35)
			.Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
			[
				SAssignNew(HighlightTextBox, SSearchBox)
				.HintText(LOCTEXT("HighlightLogHint", "Find In Log (Ctrl + F)"))
				.OnTextCommitted(this, &SLogViewerProWidgetTopBar::OnHighlightTextCommitted)
				.OnTextChanged(this, &SLogViewerProWidgetTopBar::OnHighlightTextChanged)
				.DelayChangeNotificationsWhileTyping(true)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(0.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.IsEnabled_Lambda([&]() { return !HighlightTextBox->GetText().IsEmpty(); })
				.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnFindPrevPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LogViewerProFindPrev", "<"))
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(0.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
				.IsEnabled_Lambda([&]() { return !HighlightTextBox->GetText().IsEmpty() ; })
				.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnFindNextPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LogViewerProFindNext", ">"))
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SCheckBox)
				.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked_Lambda([&]() { return MainWidget->IsFollowLog() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnFollowLogPressed)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(4.0, 2.0))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Follow Log", "Follow Log"))
					]
				]
			]

			//+ SHorizontalBox::Slot()
			//.AutoWidth()
			//.HAlign(HAlign_Right)
			//.Padding(FMargin(5.0f, 0.0f))
			//[
			//	SNew(SCheckBox)
			//	.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
			//	.IsChecked_Lambda([&]() {return MainWidget->IsShowMinimap() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			//	.OnCheckStateChanged(this, &SLogViewerProWidgetTopBar::OnMinimapPressed)
			//	[
			//		SNew(SBox)
			//		.VAlign(VAlign_Center)
			//		.HAlign(HAlign_Center)
			//		.Padding(FMargin(4.0, 2.0))
			//		[
			//			SNew(STextBlock)
			//			.Text(LOCTEXT("LogViewerProMinimap", "Minimap"))
			//		]
			//	]
			//]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 2.0f)
			[
				SettingsButton.ToSharedRef()
				//[
				
				//[					
					//SNew(SHorizontalBox)
					//
					//+ SHorizontalBox::Slot()
					//.AutoWidth()
					//.HAlign(HAlign_Right)
					//.Padding(FMargin(4.0, 2.0))
					//[
					//	SNew(STextBlock)
					//	.Text(LOCTEXT("Settings", "Settings"))
					//]
					//+ SHorizontalBox::Slot()
					//.AutoWidth()
					//.HAlign(HAlign_Right)
					//.Padding(FMargin(4.0, 2.0))
					//[
					//	SNew(SImage)
					//	.Image(FAppStyle::Get().GetBrush("Icons.Settings"))
					//	.ColorAndOpacity(FSlateColor::UseForeground())
					//]
				//]
			]
		]
	];
}

void SLogViewerProWidgetTopBar::OnListenEnginePressed(ECheckBoxState CheckState)
{
	if (!MainWidget->IsListeningEngine())
	{
		MainWidget->BeginListenEngine();
	}
	else
	{
		MainWidget->StopListenEngine();
	}
}

void SLogViewerProWidgetTopBar::OnClearOutputPressed(ECheckBoxState CheckState)
{
	if (MainWidget->LogMessages.Num() > 0)
	{
		MainWidget->CleanupMessagesOnly();
	}
	else
	{
		MainWidget->CleanupAll();
	}
}

void SLogViewerProWidgetTopBar::OnFollowLogPressed(ECheckBoxState CheckState)
{
	if (MainWidget->IsFollowLog())
	{
		MainWidget->EndFollowLog();
	}
	else
	{
		MainWidget->BeginFollowLog();
	}
}

void SLogViewerProWidgetTopBar::OnSaveFilePressed(ECheckBoxState CheckState)
{
	SaveLog();
}

void SLogViewerProWidgetTopBar::OnOpenFilePressed(ECheckBoxState CheckState)
{
	OpenLog();
}

void SLogViewerProWidgetTopBar::OpenLog()
{
	if (LastSelectedLogsPath.IsEmpty())
	{
		LastSelectedLogsPath = FPaths::ProjectLogDir();
	}

	const FString ProjectFileDescription = LOCTEXT("FileTypeDescription", "Log file").ToString();
	const FString LogFileType(TEXT("log"));
	const FString LogFileExtension = FString::Printf(TEXT("*.%s"), *LogFileType);
	const FString FileTypes = FString::Printf(TEXT("%s (%s)|%s"), *ProjectFileDescription, *LogFileExtension, *LogFileExtension);

	// Prompt the user for the filenames
	TArray<FString> OpenFilenames;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bOpened = false;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		bOpened = DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			LOCTEXT("OpenProjectBrowseTitle", "Open Project").ToString(),
			LastSelectedLogsPath,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::None,
			OpenFilenames
		);
	}

	if (!bOpened || OpenFilenames.Num() == 0)
	{
		return;
		//HandleProjectViewSelectionChanged(NULL, ESelectInfo::Direct, FText());
	}
	FString Path = OpenFilenames[0];
	if (FPaths::IsRelative(Path))
	{
		Path = FPaths::ConvertRelativePathToFull(Path);
	}
	LastSelectedLogsPath = Path;



	if (!FPaths::FileExists(Path))
	{
		return;
	}

	TArray<FString> LogStrings;
	if (!FFileHelper::LoadANSITextFileToStrings(*Path, nullptr, LogStrings) == true)
	{
		return;
	}

	if (LogStrings.Num() == 0)
	{
		return;
	}

	MainWidget->StopListenEngine();
	MainWidget->CleanupAll();
	MainWidget->ScrollTo(0);

	//@HACK - there might be leftovers of UTF-8 signature in the first line. Remove if exist
	LogStrings[0].RemoveFromStart(TEXT("???"));

	int32 LineWithTimeStartIdx = LogStrings.Num();

	//Parse lines without time
	for (int32 idx = 0; idx < LogStrings.Num(); idx++)
	{
		const FString& LogString = LogStrings[idx];
		if (IsLogLineWithTime(LogString))
		{
			LineWithTimeStartIdx = idx;
			break;
		}
		else
		{
			FLogMessage LogMsg = ParseLogLineSimple(LogString);
			LogMsg.bNoTime = true;
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
		}
	}
	// Parse each line from config
	FString AccumulatorString = FString();
	for (int32 idx = LineWithTimeStartIdx; idx < LogStrings.Num(); idx++)
	{
		const FString& LogString = LogStrings[idx];
		const bool bLastLine = idx == LogStrings.Num() - 1;
		if (IsLogLineWithTime(LogString))
		{
			const FLogMessage LogMsg = ParseLogLineWithTime(AccumulatorString);
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
			AccumulatorString = TEXT("") + LogString;
		}
		else if (bLastLine)
		{
			const FLogMessage LogMsg = ParseLogLineWithTime(AccumulatorString + LogString);
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
		}
		else
		{
			AccumulatorString += TEXT("\r\n") + LogString;
		}
	}

#if 0 //Parser 2.0

	// Parse each line from config
	bParsingLineWithTime = false;
	FString AccumulatorString = TEXT("\r\n");
	bool bMultiline = false;
	for (int32 idx = 0; idx < LogStrings.Num(); idx++)
	{
		FString& LogString = LogStrings[idx];
		LogString.TrimStartAndEndInline();
		if (!IsLogLineWithTime(LogString) && !bParsingLineWithTime)
		{
			FLogMessage LogMsg = ParseLogLineSimple(LogString);
			LogMsg.bNoTime = true;
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
			continue;
		}
		bParsingLineWithTime = true;

		const bool bLastLine = idx == LogStrings.Num() - 1;
		FString& NextString = LogStrings[idx + 1];
		if (bLastLine || IsLogLineWithTime(LogString) && IsLogLineWithTime(NextString))
		{
			bParsingLineWithTime = true; //After that point, new lines are just parts of previous log message
			FLogMessage LogMsg = ParseLogLineWithTime(LogString);
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
			continue;
		}
		else //@Dark magic to iterate new lines until next log entry
		{
			FString FullString = FString(LogString, 0) + TEXT("\r\n");
			int32 nextLineIdx = idx + 1;
			while (!IsLogLineWithTime(LogStrings[nextLineIdx]) && nextLineIdx != LogStrings.Num()-1)
			{
				FullString += LogStrings[nextLineIdx] + TEXT("\r\n");
				nextLineIdx += 1;
			}

			FLogMessage LogMsg = ParseLogLineWithTime(FullString);
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
			idx = nextLineIdx;
			continue;
		}

	}
#endif
#if 0 //Parser 1.0
	for (auto& LogString : LogStrings)
	{
		LogString.TrimStartAndEndInline();
		if (!IsLogLineWithTime(LogString) && !bParsingLineWithTime)
		{
			FLogMessage LogMsg = ParseLogLineSimple(LogString);
			LogMsg.bNoTime = true;
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));
			continue;
		}
		else if (!IsLogLineWithTime(LogString) && bParsingLineWithTime)
		{
			AccumulatorString = AccumulatorString + TEXT("\r\n") + LogString; \
			bMultiline = true;
			continue;
		}
		else if(bMultiline)
		{
			bParsingLineWithTime = true; //After that point, new lines are just parts of previous log message
			FLogMessage LogMsg = ParseLogLineWithTime(LogString);
			MainWidget->HandleLogReceived(MakeShared<FLogMessage>(LogMsg));

			continue;
		}
	}
#endif
	if (MainWidget->GetParentTab().IsValid())
	{
		MainWidget->GetParentTab().Pin()->SetLabel(FText::FromString(FPaths::GetBaseFilename(Path)));
	}

	//Default categories support for Open Files
	if (CategoryMenu->IsDefaultCategoriesFileExists())
	{
		CategoryMenu->LoadDefaultCategories();
	}
	else
	{
		CategoryMenu->ExecuteCategoriesEnableAll(ECheckBoxState::Checked);
	}
}

void SLogViewerProWidgetTopBar::Cleanup()
{
	CategoryMenu->Filter.SetFilterText(FText());
	HighlightTextBox->SetText(FText());
	FilterTextBox->SetText(FText());;
}

FLogMessage SLogViewerProWidgetTopBar::ParseLogLineSimple(const FString& LogString)
{

	const int32 CatEndPos = LogString.Find(TEXT(": "), ESearchCase::CaseSensitive, ESearchDir::FromStart);
	const int32 FirstSpacePos = LogString.Find(TEXT(" "), ESearchCase::CaseSensitive, ESearchDir::FromStart);
	if (CatEndPos == INDEX_NONE || FirstSpacePos < CatEndPos) //means no category
	{
		return FLogMessage(LogString);
	}

	const FString LogCategory = LogString.Mid(0, CatEndPos);

	const int32 VerbosityStartPos = CatEndPos + 2;
	const int32 VerbosityEndPos = LogString.Find(TEXT(": "), ESearchCase::CaseSensitive, ESearchDir::FromStart, VerbosityStartPos);
	const int32 VerbositySpacePos = LogString.Find(TEXT(" "), ESearchCase::CaseSensitive, ESearchDir::FromStart, VerbosityStartPos);
	if (VerbosityEndPos == INDEX_NONE || VerbositySpacePos < VerbosityEndPos) //means no verbosity
	{
		return FLogMessage(LogString.Mid(VerbosityStartPos), FName(*LogCategory));
	}

	const FString StringVerbosityLevel = LogString.Mid(VerbosityStartPos, VerbosityEndPos-VerbosityStartPos);
	ELogVerbosity::Type Verbosity = ELogVerbosity::Log;
	if (StringVerbosityLevel.Equals(TEXT("Fatal")))
	{
		Verbosity = ELogVerbosity::Fatal;
	}
	else if (StringVerbosityLevel.Equals(TEXT("Error")))
	{
		Verbosity = ELogVerbosity::Error;
	}
	else if (StringVerbosityLevel.Equals(TEXT("Warning")))
	{
		Verbosity = ELogVerbosity::Warning;
	}
	else if (StringVerbosityLevel.Equals(TEXT("Display")))
	{
		Verbosity = ELogVerbosity::Display;
	}
	else if (StringVerbosityLevel.Equals(TEXT("Verbose")))
	{
		Verbosity = ELogVerbosity::Verbose;
	}
	else if (StringVerbosityLevel.Equals(TEXT("VeryVerbose")))
	{
		Verbosity = ELogVerbosity::VeryVerbose;
	}
	else //just word with ":" in the end
	{
		return FLogMessage(LogString.Mid(VerbosityStartPos), FName(*LogCategory));
	}
	const int32 MsgStartPos = VerbosityEndPos + 2;
	return FLogMessage(LogString.Mid(MsgStartPos), Verbosity, FName(*LogCategory));
}

FLogMessage SLogViewerProWidgetTopBar::ParseLogLineWithTime(const FString& LogString)
{
	const int32 TimeEndPos = LogString.Find(TEXT("]["), ESearchCase::CaseSensitive, ESearchDir::FromStart);
	if (TimeEndPos == INDEX_NONE) //Very unexpected, @todo if i have time add some logging and request broken log file from the user
	{
		return FLogMessage(TEXT("Was unable to parse the message, please report log file to d.labadin@gmail.com"));
	}
	const int32 FrameStartPos = TimeEndPos + 2;
	const int32 FrameEndPos = LogString.Find(TEXT("]"), ESearchCase::CaseSensitive, ESearchDir::FromStart, FrameStartPos);
	
	const FString SimpleString = LogString.Mid(FrameEndPos + 1);
	FLogMessage LogMessage = ParseLogLineSimple(SimpleString);
	
	FDateTime::Parse(LogString.Mid(1, TimeEndPos-1), LogMessage.Time);

	int32 Frame = FCString::Atoi(*LogString.Mid(FrameStartPos, FrameEndPos-FrameStartPos));
	LogMessage.FrameNumber = Frame;
	return LogMessage;
}

bool SLogViewerProWidgetTopBar::IsLogLineWithTime(const FString& LogString) const
{
	int32 pos = INDEX_NONE;
	LogString.FindChar('[', pos);
	return pos == 0;
}

void SLogViewerProWidgetTopBar::OnFilterTextChanged(const FText& InFilterText)
{
	if (InFilterText.IsEmpty())
	{
		OnFilterTextCommitted(InFilterText, ETextCommit::OnCleared);
	}
}

void SLogViewerProWidgetTopBar::OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	CategoryMenu->Filter.SetFilterText(InFilterText);
	FilterTextBox->SetError(CategoryMenu->Filter.GetSyntaxErrors());
	MainWidget->Refresh();
}

void SLogViewerProWidgetTopBar::OnHighlightTextChanged(const FText& InFilterText)
{
	if (InFilterText.IsEmpty())
	{
		OnHighlightTextCommitted(InFilterText, ETextCommit::OnCleared);
	}
}

void SLogViewerProWidgetTopBar::OnHighlightTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	//Mega logic, because UE clear button in the text box is not Commit, but only TextChanged to ""
	//because of that I have to support custom behavior in case when text was cleared and not refocus to ListView in the MainWidget
	const bool bFocusToList = InCommitType == ETextCommit::OnCleared ? false : true; 

	MainWidget->SetHighlightText(InFilterText, bFocusToList);
	MainWidget->Refresh();
}

void SLogViewerProWidgetTopBar::OnFindNextPressed(ECheckBoxState CheckState)
{
	MainWidget->FindNext();
}

void SLogViewerProWidgetTopBar::OnFindPrevPressed(ECheckBoxState CheckState)
{
	MainWidget->FindPrevious();
}

void SLogViewerProWidgetTopBar::SaveLog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "Saving is not supported on this platform!"));

		return;
	}

	TArray<FString> Filenames;

	// open file dialog
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	void* ParentWindowHandle = (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid()) ? ParentWindow->GetNativeWindow()->GetOSWindowHandle() : nullptr;
	
	if (!DesktopPlatform->SaveFileDialog(
		ParentWindowHandle,
		LOCTEXT("SaveLogDialogTitle", "Save Log As...").ToString(),
		LastLogFileSaveDirectory,
		FString::Printf(TEXT("LogViewerProOutput-%s.log"), *FDateTime::Now().ToString()),
		//TEXT("LogViewerProOutput-", FDateTime::Now().ToString(), ".log"),
		TEXT("Log Files (*.log)|*.log"),
		EFileDialogFlags::None,
		Filenames))
	{
		return;
	}

	// no log file selected?
	if (Filenames.Num() == 0)
	{
		return;
	}

	FString Filename = Filenames[0];

	// keep path as default for next time
	LastLogFileSaveDirectory = FPaths::GetPath(Filename);

	// add a file extension if none was provided
	if (FPaths::GetExtension(Filename).IsEmpty())
	{
		Filename += Filename + TEXT(".log");
	}

	// save file
	FArchive* LogFile = IFileManager::Get().CreateFileWriter(*Filename);

	if (LogFile != nullptr)
	{
		for (const auto& LogMessage : MainWidget->LogMessages)
		{
			FString LogEntry = LogMessage->AsString(true) + LINE_TERMINATOR;
			LogFile->Serialize(TCHAR_TO_ANSI(*LogEntry), LogEntry.Len());
		}

		LogFile->Close();
		delete LogFile;
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogFileError", "Failed to open the specified file for saving!"));
	}
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE