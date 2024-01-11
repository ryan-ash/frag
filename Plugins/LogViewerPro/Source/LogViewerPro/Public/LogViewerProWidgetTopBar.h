
// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SlateFwd.h"
#include "Brushes/SlateColorBrush.h"
#include "LogViewerProStructs.h"

class FOutputLogTextLayoutMarshaller;
class SLogViewerProWidgetCategoriesView;
class SLogViewerProWidgetMain;
class SLogViewerProSettingsButton;
class SSearchBox;

class SLogViewerProWidgetTopBar
	: public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SLogViewerProWidgetTopBar)
		: _MainWidget()
		, _CategoryMenu()
	{}
		SLATE_ARGUMENT(SLogViewerProWidgetMain*, MainWidget)
		SLATE_ARGUMENT(SLogViewerProWidgetCategoriesView*, CategoryMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

		SLogViewerProWidgetMain* MainWidget;
	SLogViewerProWidgetCategoriesView* CategoryMenu;

	TSharedPtr< SSearchBox > FilterTextBox;
	TSharedPtr< SSearchBox > HighlightTextBox;

	TSharedPtr< STextBlock > ClearOutputCaption;
	
	/** The button that displays view options */
	//TSharedPtr<SComboButton> ViewOptionsComboButton;


	/** Handler for when the view combo button is clicked */
	void SaveLog();
	void OpenLog();
	void Cleanup();

	void HighlightDesiredText(const FText& InFilterText, bool bFocusToList);

	SLogViewerProSettingsButton* GetSettings() const {return SettingsButton.Get(); };
private:

	TSharedPtr<SLogViewerProSettingsButton> SettingsButton;

	//FSlateColorBrush brushClr;// = FSlateColorBrush(FLinearColor::White);
	void OnListenEnginePressed(ECheckBoxState CheckState);
	void OnClearOutputPressed(ECheckBoxState CheckState);
	void OnOpenFilePressed(ECheckBoxState CheckState);
	void OnSaveFilePressed(ECheckBoxState CheckState);
	void OnFollowLogPressed(ECheckBoxState CheckState);
	FString LastSelectedLogsPath;
	FLogMessage ParseLogLineSimple(const FString& LogString);
	FLogMessage ParseLogLineWithTime(const FString& LogString);
	bool IsLogLineWithTime(const FString& LogString) const;
	bool bParsingLineWithTime = false;

	void OnFilterTextChanged(const FText& InFilterText);
	void OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);
	void OnHighlightTextChanged(const FText& InFilterText);
	void OnHighlightTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);

	void OnFindNextPressed(ECheckBoxState CheckState);
	void OnFindPrevPressed(ECheckBoxState CheckState);

	FString LastLogFileSaveDirectory;
};
