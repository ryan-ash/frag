// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "LogViewerProOutputDevice.h"
#include "Widgets/Views/SListView.h"

class FOutputLogTextLayoutMarshaller;
class SMultiLineEditableTextBox;
class SLogViewerProWidgetCategoriesView;
class SLogViewerProWidgetMinimap;
class SLogViewerProWidgetTopBar;
class SLogViewerProSettingsButton;
class FTabManager;
class SDockTab;
class FLogViewerProModule;

class SLogViewerProWidgetMain 
	: public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS( SLogViewerProWidgetMain )
		{}
		SLATE_ARGUMENT(TSharedPtr<FLogViewerProOutputDevice>, LogOutputDevice)
		SLATE_ARGUMENT(FLogViewerProModule*, Module)
	SLATE_END_ARGS()

	virtual ~SLogViewerProWidgetMain();

	void Construct( const FArguments& InArgs );
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void BeginListenEngine();
	void StopListenEngine();
	bool IsListeningEngine() const;
private:
	FDelegateHandle ListeningDelegate;
public:

	TSharedPtr< FLogViewerProOutputDevice > LogDevice;

	void HandleLogReceived(TSharedPtr<FLogMessage> Message);

	/** The editable text showing all log messages */
	TSharedPtr< SLogViewerProWidgetCategoriesView > CategoryMenu;
	TSharedPtr< SLogViewerProWidgetMinimap > Minimap;
	TSharedPtr< SLogViewerProWidgetTopBar > TopBar;
	/** The editable text showing all log messages */
	TSharedPtr< SSearchBox > FilterTextBox;


	TSharedPtr<SListView<TSharedPtr<FLogMessage>>> LogListView;

	TSharedPtr<SHorizontalBox> ListBox;
	TSharedPtr<SBorder> ListBorder;

	SLogViewerProSettingsButton* GetSettings() const;

	/** Forces re-population of the messages list */
	void Refresh();

	void CleanupAll();
	void CleanupMessagesOnly();
	void CleanUpFilters();

	void SelectAllByCategory(FName Category);

	void SelectAllActiveLogs();
	void ScrollDown();
	void ScrollTo(int32 LineNumber);
	void ScrollRelative(float offset);
	void BeginFollowLog();
	void EndFollowLog();
	bool IsFollowLog() const;
	void OnUserScrolled(double ScrollOffset);
	void OnSelectionChanged(TSharedPtr<FLogMessage> Selection, ESelectInfo::Type SelectInfo);

	void SetParentTab(TWeakPtr<SDockTab> InParentTab) { ParentTab = InParentTab; };
	TWeakPtr<SDockTab> GetParentTab() const { return ParentTab; };

	TArray<TSharedPtr<FLogMessage>> LogMessages;

	FText HandleLogListGetHighlightText() const;
	void SetHighlightText(const FText& InText, bool bFocusOnTheList);
	void UpdateShowTime();
	void UpdateShowMinimap();

	void FindNext();
	void FindPrevious();

	//void SetLogViewerProModule(FLogViewerProModule* InLogViewerProModule) { LogViewerProModule = InLogViewerProModule; };
	FLogViewerProModule* GetLogViewerProModule() const { return LogViewerProModule; };
	FLogColorTable* GetColorTable() const { return LogColorTable.Get(); };

	void CopySelectedLogs();

	void DeleteSelectedMessages(); //@TODO

private:

	FLogViewerProModule* LogViewerProModule;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	FText HighlightText;
	int32 FoundLocation = INDEX_NONE;
	const int32 INDEX_SEARCH_FINISHED = -2;
	const FName CATEGORY_BLUEPRINT = FName("LogBlueprintUserMessages");

	/** Holds the log list view. */

	/** Holds an unfiltered list of available log messages. */

	TSharedPtr< SMultiLineEditableTextBox > DetailedMessageView;

	TArray<TSharedPtr<FLogMessage>> AvailableLogs;

	void ReloadLog();

	TSharedRef<ITableRow> HandleLogListGenerateRow(TSharedPtr<FLogMessage> Message, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleLogListItemScrolledIntoView(TSharedPtr<FLogMessage> Item, const TSharedPtr<ITableRow>& TableRow);

	TWeakPtr<SDockTab> ParentTab;


	void SetFocusToHighlightEditBox();
	void SetFocusToFilterEditBox();
	void SetFocusToFilterCategoryBox();
	void ClearSelection();


	bool bIsFollowLog;

	void HandlePreBeginPIE(bool bIsSimulating);
	void HandleBeginPIE(bool bIsSimulating);

	float GetListContainerWidth() const;
	const float WidthMinimap = 0.05f;

	TUniquePtr< FLogColorTable > LogColorTable;
};
