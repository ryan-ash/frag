// Copyright Dmitrii Labadin 2019

#include "LogViewerProWidgetMain.h"

#include "SlateOptMacros.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Misc/CoreDelegates.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"

#include "HAL/PlatformApplicationMisc.h"

#include "LogViewerProStructs.h"
#include "LogViewerProWidgetCategoriesView.h"
#include "LogViewerProWidgetMinimap.h"
#include "LogViewerProWidgetTopBar.h"
#include "LogViewerProSettingsButton.h"
#include "Engine/Engine.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "SLogViewerProTableRow.h"
#include "Framework/Application/SlateApplication.h"
#include "DrawDebugHelpers.h"
#include "LogViewerPro.h"

#include "Widgets/Input/SSearchBox.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

static const FName ColumnTime = FName("Time");
static const float ColumnTimeWidth = 0.06f;
static const FName ColumnSeconds = FName("TimeSeconds");
static const float ColumnSecondsWidth = 0.04f;
static const FName ColumnFrame = FName("Frame");
static const float ColumnFrameWidth = 0.03f;


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLogViewerProWidgetMain::Construct( const FArguments& InArgs )
{
	FEditorDelegates::PreBeginPIE.AddRaw(this, &SLogViewerProWidgetMain::HandleBeginPIE);

	LogDevice = InArgs._LogOutputDevice;
	LogViewerProModule = InArgs._Module;
	
	LogColorTable = MakeUnique<FLogColorTable>();

	CategoryMenu = SNew(SLogViewerProWidgetCategoriesView)
		.MainWidget(this);	

	TopBar = SNew(SLogViewerProWidgetTopBar)
		.MainWidget(this)
		.CategoryMenu(CategoryMenu.Get());

	Minimap = SNew(SLogViewerProWidgetMinimap)
		.MainWidget(this);

	DetailedMessageView = SNew(SMultiLineEditableTextBox)
		.Style(FAppStyle::Get(), "Log.TextBox")
		.TextStyle(FAppStyle::Get(), "Log.Normal")
		.ForegroundColor(FLinearColor::Gray)
		.AutoWrapText(true)
		.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
		//.Marshaller(MessagesTextMarshaller)
		//.OnVScrollBarUserScrolled(this, &SLogViewerProWidgetMain::OnUserScrolled)
		.IsReadOnly(true)
		//.AlwaysShowScrollbars(true);
		;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(3)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		//.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Output Log Filter
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0)
			.FillHeight(1.0)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.8f)
				[
					SNew(SVerticalBox)
					// Output Log Filter
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					[
						TopBar.ToSharedRef()
					]

					+ SVerticalBox::Slot()
					.FillHeight(0.95f)
					[
						SAssignNew(ListBox, SHorizontalBox)

						+ SHorizontalBox::Slot()
						//.FillWidth(1-WidthMinimap)
						[
							// log list
							SAssignNew(ListBorder, SBorder)
							//.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
							.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
							.Padding(0.0f)
							.ForegroundColor(FLinearColor::Gray)
							[
								SAssignNew(LogListView, SListView<TSharedPtr<FLogMessage>>)
									.ItemHeight(24.0f)
									.ListItemsSource(&LogMessages)
									.SelectionMode(ESelectionMode::Multi)
									.OnGenerateRow(this, &SLogViewerProWidgetMain::HandleLogListGenerateRow)
									.OnItemScrolledIntoView(this, &SLogViewerProWidgetMain::HandleLogListItemScrolledIntoView)
									.OnListViewScrolled(this, &SLogViewerProWidgetMain::OnUserScrolled)
									.OnSelectionChanged(this, &SLogViewerProWidgetMain::OnSelectionChanged)
									.HeaderRow
									(
										SNew(SHeaderRow)

										+ SHeaderRow::Column("Verbosity")
											.DefaultLabel(LOCTEXT("LogViewerProVerbosity", " "))
											.FixedWidth(24.0f)

										+ SHeaderRow::Column(ColumnTime)
											.DefaultLabel(LOCTEXT("LogViewerProTime", "Time"))
											.FillWidth( GetSettings()->IsShowTime() ? ColumnTimeWidth : 0.f)
											.HAlignHeader(EHorizontalAlignment::HAlign_Right)
											.HAlignCell(EHorizontalAlignment::HAlign_Right)

										+ SHeaderRow::Column(ColumnSeconds)
											.DefaultLabel(LOCTEXT("LogViewerProSeconds", "Seconds"))
											.FillWidth(GetSettings()->IsShowTime() ? ColumnSecondsWidth : 0.f)
											.HAlignHeader(EHorizontalAlignment::HAlign_Right)
											.HAlignCell(EHorizontalAlignment::HAlign_Right)

										+ SHeaderRow::Column(ColumnFrame)
											.DefaultLabel(LOCTEXT("LogViewerProFrame", "Frame"))
											.FillWidth(GetSettings()->IsShowTime() ? ColumnFrameWidth : 0.f)
											.HAlignHeader(EHorizontalAlignment::HAlign_Right)
											.HAlignCell(EHorizontalAlignment::HAlign_Right)

										+ SHeaderRow::Column("Category")
											.DefaultLabel(LOCTEXT("LogViewerProCategory", "Category"))
											.FillWidth(0.10f)

										+ SHeaderRow::Column("Message")
											.DefaultLabel(LOCTEXT("LogViewerProMessage", "Message"))
											.FillWidth(0.70f)
									)
							]
						]

						+ SHorizontalBox::Slot()
						.FillWidth(WidthMinimap)
						.Padding(5.0f, 25.0f, 5.0f, 5.0f)
						[
							Minimap.ToSharedRef()
						]
					]

					+ SVerticalBox::Slot()
					.Padding(0.f, 10.0f, 0.f, .0f)
					.AutoHeight()
					[
						DetailedMessageView.ToSharedRef()
					]
				]
			
				+SHorizontalBox::Slot()
				.FillWidth(0.2f)
				[
					CategoryMenu.ToSharedRef()
				]
			]

			
		]
	];

	const bool bActivateCategory = !CategoryMenu->IsDefaultCategoriesFileExists();

	CategoryMenu->Init();

	GetSettings()->TryLoadAndApplySettings();

	BeginListenEngine();
}

void SLogViewerProWidgetMain::HandleBeginPIE(bool bIsSimulating)
{
	if(IsListeningEngine() && GetSettings()->IsCleanUpLogsOnPie())
	{
		this->CleanupMessagesOnly();
	}
}

void SLogViewerProWidgetMain::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Now processing of buffered lines must be triggered from slate thread, not from any thread as it was before
	LogDevice->ProcessBufferedLines();
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

SLogViewerProWidgetMain::~SLogViewerProWidgetMain()
{
	FEditorDelegates::PreBeginPIE.RemoveAll(this);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerProWidgetMain::BeginListenEngine()
{
	//FOnLogMsgReceivedDelegate LogDelegate = FOnLogMsgReceivedDelegate::CreateRaw<SLogViewerProWidgetMain>(this, &SLogViewerProWidgetMain::HandleLogReceived);
	//LogDevice->SubscribeOnLogReceived(LogDelegate);
	ListeningDelegate = LogDevice->SubscribeOnMessageReceived(this);
	BeginFollowLog();
}

void SLogViewerProWidgetMain::StopListenEngine()
{
	LogDevice->UnsibscribeOnMessageReceived(ListeningDelegate);
}

bool SLogViewerProWidgetMain::IsListeningEngine() const
{
	return LogDevice->IsListening(this);
}

void SLogViewerProWidgetMain::HandleLogReceived(TSharedPtr<FLogMessage> Message)
{
	const bool bShouldEnableCategory = !GetLogViewerProModule()->IsDefaultCategoriesEnabled();
	CategoryMenu->AddCategory(Message->Category, bShouldEnableCategory);

	AvailableLogs.Add(Message);

	if (CategoryMenu->FilterLogMessage(Message.ToSharedRef()))
	{
		LogMessages.Add(Message);
	}

	LogListView->RequestListRefresh();

	if (GetSettings()->IsShowMinimap())
	{
		Minimap->RequestUpdate();
	}

	if (bIsFollowLog)
	{
		ScrollDown();
	}

	//Autohighligh logic
	if (!TopBar->HighlightTextBox->GetText().IsEmpty() && Message->Message->Contains(TopBar->HighlightTextBox->GetText().ToString()))
	{
		Message->bFound = true;
	}

	//Categorize messages from Blueprint's print String
	if (Message->Category.IsEqual(CATEGORY_BLUEPRINT))
	{
		if (Message->Message->Contains(TEXT("Warning:")))
		{
			Message->Verbosity = ELogVerbosity::Warning;
		}
		else if (Message->Message->Contains(TEXT("Error:")))
		{
			Message->Verbosity = ELogVerbosity::Error;
		}

	}

}

void SLogViewerProWidgetMain::BeginFollowLog()
{
	bIsFollowLog = true;
	ScrollDown();
}

void SLogViewerProWidgetMain::ScrollTo(int32 LineNumber)
{
	if (LineNumber <= 0)
	{
		return;
	}
	if (LogMessages.Num() == 0)
	{
		return;
	}
	const int32 Pos = FMath::Min(LineNumber, LogMessages.Num());
	if (!LogMessages.IsValidIndex(Pos))
	{
		return;
	}
	if (LineNumber > LogMessages.Num())
	{
		LogListView->RequestScrollIntoView(LogMessages.Last());
		return;
	}
	LogListView->RequestScrollIntoView(LogMessages[Pos]);
	EndFollowLog();
}



void SLogViewerProWidgetMain::ScrollRelative(float offset)
{
	LogListView->AddScrollOffset(offset, true);
	EndFollowLog();
}

void SLogViewerProWidgetMain::ScrollDown()
{
	if(LogMessages.Num() != 0)
	{
		LogListView->RequestScrollIntoView(LogMessages.Last());
	}
}

void SLogViewerProWidgetMain::EndFollowLog()
{
	bIsFollowLog = false;
}

bool SLogViewerProWidgetMain::IsFollowLog() const
{
	return bIsFollowLog;
}

void SLogViewerProWidgetMain::OnUserScrolled(double ScrollOffset)
{
	EndFollowLog();
	Minimap->RequestUpdate();
}

void SLogViewerProWidgetMain::OnSelectionChanged( TSharedPtr<FLogMessage> Selection, ESelectInfo::Type SelectInfo)
{
	Minimap->RequestUpdate();

	if (!Selection.IsValid())
	{
		DetailedMessageView->SetText(FText());
		return;
	}
	
	DetailedMessageView->SetText(FText::FromString(Selection->AsString(true)));
}

SLogViewerProSettingsButton* SLogViewerProWidgetMain::GetSettings() const
{
	return TopBar->GetSettings();
}

void SLogViewerProWidgetMain::Refresh()
{
	CategoryMenu->MarkDirty();
	ReloadLog();
	Minimap->RequestUpdate();
	//RequestForceScroll();
}

void SLogViewerProWidgetMain::CleanupAll()
{
	DetailedMessageView->SetText(FText());

	CategoryMenu->ClearCategories();
	TopBar->Cleanup();
	LogMessages.Reset();
	AvailableLogs.Reset();
	SetHighlightText(FText(), false);
	Refresh();
}

void SLogViewerProWidgetMain::CleanupMessagesOnly()
{
	DetailedMessageView->SetText(FText());

	TopBar->Cleanup();
	LogMessages.Reset();
	AvailableLogs.Reset();
	SetHighlightText(FText(), false);
	Refresh();
}

void SLogViewerProWidgetMain::SelectAllByCategory(FName Category)
{
	bool Select = false; //By default deselect

	TArray< TSharedPtr<FLogMessage>> MessagesToSelect;
	for (auto& LogMessage : LogMessages)
	{
		if (LogMessage->Category != Category)
		{
			continue;
		}
		MessagesToSelect.Add(LogMessage);

		if (!LogListView->IsItemSelected(LogMessage)) //if at least one message is not selected, then select all, n^2 difficulty?, I am sorry
		{
			Select = true;
		}
	}

	LogListView->SetItemSelection(MessagesToSelect, Select, ESelectInfo::OnMouseClick);
}

void SLogViewerProWidgetMain::SelectAllActiveLogs()
{
	LogListView->SetItemSelection(LogMessages, true, ESelectInfo::OnMouseClick);
}

TSharedRef<ITableRow> SLogViewerProWidgetMain::HandleLogListGenerateRow(TSharedPtr<FLogMessage> Message, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SLogViewerProTableRow, OwnerTable)
		.HighlightText(this, &SLogViewerProWidgetMain::HandleLogListGetHighlightText)
		.LogMessage(Message)
		.MainWidget(this)
		//.ToolTipText(FText::FromString(Message->Text));
		//.Style(FEditorStyle::Get(), "Log.TextBox")
		;
}

FText SLogViewerProWidgetMain::HandleLogListGetHighlightText() const
{
	return HighlightText; //FilterBar->GetFilterText();
}

void SLogViewerProWidgetMain::SetHighlightText(const FText& InText, bool bFocusOnTheList)
{
	HighlightText = InText;
	FoundLocation = INDEX_NONE;
	if (bFocusOnTheList)
	{
		FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), LogListView->AsShared());
	}
	Refresh();
}

void SLogViewerProWidgetMain::UpdateShowTime()
{
	LogListView->GetHeaderRow()->SetColumnWidth(ColumnTime, GetSettings()->IsShowTime() ? ColumnTimeWidth :  0.f );
	LogListView->GetHeaderRow()->SetColumnWidth(ColumnSeconds, GetSettings()->IsShowSeconds() ? ColumnSecondsWidth : 0.f);
	LogListView->GetHeaderRow()->SetColumnWidth(ColumnFrame, GetSettings()->IsShowFrame() ? ColumnFrameWidth : 0.f);
}

void SLogViewerProWidgetMain::UpdateShowMinimap()
{
	Minimap->SetVisibility(GetSettings()->IsShowMinimap() ? EVisibility::Visible : EVisibility::Collapsed);
	if(Minimap->GetVisibility() != EVisibility::Collapsed)
	{
		Minimap->RequestUpdate();
	}
	//if (IsShowMinimap())
	//{
	//	Minimap->GenerateBrush();
	//}
}

void SLogViewerProWidgetMain::HandleLogListItemScrolledIntoView(TSharedPtr<FLogMessage> Item, const TSharedPtr<ITableRow>& TableRow)
{
	if (LogMessages.Num() > 0)
	{
		bIsFollowLog = LogListView->IsItemVisible(LogMessages.Last());
	}
	else
	{
		bIsFollowLog = true;
	}
}

void SLogViewerProWidgetMain::ReloadLog()
{
	// reload log list
	LogMessages.Reset();

	FString HighlightTextStr = HighlightText.ToString();

	for (const auto& LogMessage : AvailableLogs)
	{
		if (CategoryMenu->FilterLogMessage(LogMessage.ToSharedRef()))
		{
			if (!HighlightText.IsEmpty() && (LogMessage->Message->Contains(HighlightTextStr) || LogMessage->Category.ToString().Contains(HighlightTextStr))) // Search
			{
				LogMessage->bFound = true;
			}
			else
			{
				LogMessage->bFound = false;
			}

			LogMessages.Add(LogMessage);
		}
	}

	// refresh list view
	LogListView->RequestListRefresh();
	if (IsFollowLog())
	{
		ScrollDown();
	}
}

void SLogViewerProWidgetMain::CopySelectedLogs()
{
	TArray<TSharedPtr<FLogMessage>> SelectedItems = LogListView->GetSelectedItems();

	if (SelectedItems.Num() == 0)
	{
		return;
	}

	FString SelectedText;

	for (const auto& Item : SelectedItems)
	{
		SelectedText += Item->AsString(GetSettings()->IsShowTime());
		SelectedText += LINE_TERMINATOR;
	}

	FPlatformApplicationMisc::ClipboardCopy(*SelectedText);
}

void SLogViewerProWidgetMain::SetFocusToHighlightEditBox()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), TopBar->HighlightTextBox->AsShared());
}

void SLogViewerProWidgetMain::SetFocusToFilterEditBox()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), TopBar->FilterTextBox->AsShared());
}

void SLogViewerProWidgetMain::SetFocusToFilterCategoryBox()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), CategoryMenu->SearchBoxPtr->AsShared());
}

void SLogViewerProWidgetMain::FindNext()
{
	int32 StartSearchLocation = INDEX_NONE; 
	
	if (FoundLocation == INDEX_NONE)
	{
		StartSearchLocation = FMath::FloorToInt(LogListView->GetScrollOffset());
		StartSearchLocation = FMath::Max(0, FoundLocation);
	}
	else if (FoundLocation == INDEX_SEARCH_FINISHED)
	{
		StartSearchLocation = 0;
	}
	else
	{
		StartSearchLocation = FoundLocation + 1;
	}

	if (!LogMessages.IsValidIndex(StartSearchLocation))
	{
		return;
	}

	for (int32 Idx = StartSearchLocation; Idx < LogMessages.Num(); Idx++)
	{
		if (LogMessages[Idx]->bFound)
		{
			FoundLocation = Idx;
			if(FoundLocation == 0 && LogListView) //@For some reason scroll to 0 doesn't work
			{
				LogListView->ScrollToTop();
			}
			else
			{
				ScrollTo(FoundLocation);
			}

			LogListView->SetSelection(LogMessages[Idx], ESelectInfo::OnNavigation);
			break;
		}
		
		LogListView->SetSelection(nullptr, ESelectInfo::OnNavigation);
		FoundLocation = INDEX_SEARCH_FINISHED;
	}
}

void SLogViewerProWidgetMain::FindPrevious()
{
	int32 StartSearchLocation = INDEX_NONE;

	if (FoundLocation == INDEX_NONE)
	{
		StartSearchLocation = FMath::FloorToInt(LogListView->GetScrollOffset());
		StartSearchLocation = FMath::Max(0, FoundLocation);
	}
	else if (FoundLocation == INDEX_SEARCH_FINISHED)
	{
		StartSearchLocation = LogMessages.Num() - 1;
	}
	else
	{
		StartSearchLocation = FoundLocation - 1;
	}

	if (!LogMessages.IsValidIndex(StartSearchLocation))
	{
		return;
	}

	for (int32 Idx = StartSearchLocation; Idx > 0; Idx--)
	{
		if (LogMessages[Idx]->bFound)
		{
			FoundLocation = Idx;
			if (FoundLocation == 0 && LogListView) //@For some reason scroll to 0 doesn't work
			{
				LogListView->ScrollToTop();
			}
			else
			{
				ScrollTo(FoundLocation);
			}
			LogListView->SetSelection(LogMessages[Idx], ESelectInfo::OnNavigation);
			break;
		}

		LogListView->SetSelection(nullptr, ESelectInfo::OnNavigation);
		FoundLocation = INDEX_SEARCH_FINISHED;
	}
}

FReply SLogViewerProWidgetMain::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.IsControlDown())
	{
		if (InKeyEvent.GetKey() == EKeys::C)
		{
			CopySelectedLogs();
			return FReply::Handled();
		}

		if (InKeyEvent.GetKey() == EKeys::S)
		{
			TopBar->SaveLog();
			return FReply::Handled();
		}

		if (InKeyEvent.GetKey() == EKeys::O)
		{
			TopBar->OpenLog();
			return FReply::Handled();
		}

		if (InKeyEvent.GetKey() == EKeys::F)
		{
			SetFocusToHighlightEditBox();
			return FReply::Handled();
		}
	}

	if (InKeyEvent.IsShiftDown())
	{
		if (InKeyEvent.GetKey() == EKeys::F3)
		{
			FindPrevious();
			return FReply::Handled();
		}
	}

	if (InKeyEvent.IsAltDown())
	{
		if (InKeyEvent.GetKey() == EKeys::F)
		{
			SetFocusToFilterEditBox();
			return FReply::Handled();
		}

		if (InKeyEvent.GetKey() == EKeys::C)
		{
			SetFocusToFilterCategoryBox();
			return FReply::Handled();
		}
	}

	if (InKeyEvent.GetKey() == EKeys::F3)
	{
		FindNext();
		return FReply::Handled();
	}

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ClearSelection();
		return FReply::Handled();
	}

	//@TODO
	if (InKeyEvent.GetKey() == EKeys::Delete)
	{
		DeleteSelectedMessages();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}


void SLogViewerProWidgetMain::ClearSelection()
{
	LogListView->ClearSelection();
}

void SLogViewerProWidgetMain::DeleteSelectedMessages()
{
	TArray<TSharedPtr<FLogMessage>> SelectedMessages;
	LogListView->GetSelectedItems(SelectedMessages);
	LogListView->ClearSelection();
	for (auto& Message : SelectedMessages)
	{
		LogMessages.Remove(Message);
		AvailableLogs.Remove(Message);
	}
	LogListView->RebuildList();
	LogListView->RequestListRefresh();
	Refresh();
}

float SLogViewerProWidgetMain::GetListContainerWidth() const
{
	return GetSettings()->IsShowMinimap() ? 1.f - WidthMinimap : 1.f;
}

#undef LOCTEXT_NAMESPACE