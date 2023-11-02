// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/SCompoundWidget.h"
#include "LogFilter.h"
#include "Misc/TextFilter.h"

class SLogViewerProWidgetCategoriesView;
class SLogViewerProWidgetMain;

class SLogViewerProWidgetCategoriesView
	: public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SLogViewerProWidgetCategoriesView)
		: _MainWidget()
		{}

		SLATE_ARGUMENT(SLogViewerProWidgetMain*, MainWidget)
	SLATE_END_ARGS()
	~SLogViewerProWidgetCategoriesView();

	void Construct(const FArguments& InArgs);
	void Init();
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void AddCategory(const FName& Category, bool bSelected);
	bool IsSearchActive() const;
	void MarkDirty() { bNeedsUpdate = true; };
	void ClearCategories();


	FLogFilter Filter;

	bool FilterLogMessage(const TSharedRef<FLogMessage>& LogMessage);

	FSlateBrush DeselectedImage;
	FSlateBrush HoveredImage;
	FSlateBrush HoveredSelectedImage;
	FSlateBrush SelectedImage;

	FSlateBrush CategoryHighlightImage;

public:
	void ExecuteVerbosityVeryVerbose(ECheckBoxState CheckState);
	void ExecuteVerbosityVerbose(ECheckBoxState CheckState);
	void ExecuteVerbosityLog(ECheckBoxState CheckState);
	void ExecuteVerbosityDisplay(ECheckBoxState CheckState);
	void ExecuteVerbosityWarning(ECheckBoxState CheckState);
	void ExecuteVerbosityError(ECheckBoxState CheckState);

	void ExecuteCategoriesDisableAll(ECheckBoxState CheckState);
	void ExecuteCategoriesEnableAll(ECheckBoxState CheckState);

	TSharedPtr<SSearchBox> SearchBoxPtr;

	bool IsDefaultCategoriesFileExists() const;
private:
	void UpdateFilteredItems();
	bool bNeedsUpdate; //We are using boolean flag and tick function to prevent more than one update in tick

	typedef TTextFilter<const FCategoryItem&> FCategoriesEntryTextFilter;
	TSharedPtr<FCategoriesEntryTextFilter> SearchTextFilter;

	/** Called when the search text changes */
	void OnSearchChanged(const FText& InFilterText);
	void OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);

	void OnSaveCategoriesPressed(ECheckBoxState CheckState);
	void OnLoadCategoriesPressed(ECheckBoxState CheckState);
	void OnMakeDefaultCategoriesPressed(ECheckBoxState CheckState);

	TSharedPtr<SBox> DataDrivenContent;
	TSharedPtr<SListView<TSharedPtr<FCategoryItem>>> ListView;
	TSharedRef<ITableRow> OnGenerateWidgetForItem(TSharedPtr<FCategoryItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	SLogViewerProWidgetMain* MainWidget;

	ECheckBoxState IsDefaultCategoriesEnabled() const;

	ECheckBoxState IsCheckedVerbosityVeryVerbose() const;
	ECheckBoxState IsCheckedVerbosityVerbose() const;
	ECheckBoxState IsCheckedVerbosityLog() const;
	ECheckBoxState IsCheckedVerbosityDisplay() const;
	ECheckBoxState IsCheckedVerbosityWarning() const;
	ECheckBoxState IsCheckedVerbosityError() const;
	ECheckBoxState IsCheckedCategoriesShowAll() const;
	ECheckBoxState CategoriesSingle_IsChecked(FName InName) const;

	void SaveSelectedCategories();
	void LoadSelectedCategories();
	
	void ClearDefaultCategories();

	//Cleaner Save/Load/Default
	void SaveDefaultCategorties();
	bool CreateDefaultConfigFolder();
	void SaveCategoriesIntoFile(const FString& Filename);

	void LoadDefaultCategoriesOLD();
	
	bool ParseCategoriesFile(const FString& Path, FLVCategoriesSave& CategoriesSave);
	bool ParseCategoriesFileOld(const FString& Path, FLVCategoriesSave& CategoriesSave);
	void ApplyCategoriesSave(const FLVCategoriesSave& CategoriesSave);

public: // :P
	void LoadDefaultCategories();
private:

	FString GetLogViewerConfigPath() const;
	FString GetDefaultCategoriesFilePath() const;

	FString LastCategoriesFileSaveDirectory;


	
};

class SLogViewerProWidgetCategoriesEntry : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLogViewerProWidgetCategoriesEntry)
		: _MainWidget()
		, _CategoryWidget()
	{}
		SLATE_ARGUMENT(SLogViewerProWidgetMain*, MainWidget)
		SLATE_ARGUMENT(SLogViewerProWidgetCategoriesView*, CategoryWidget)

		/** Menu extender for the right-click context menu */
		SLATE_EVENT(FMenuExtensionDelegate, ContextMenuExtender)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<const FCategoryItem>& InItem);
		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	FReply ColorBlock_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);


	bool IsPressed() const;
	TSharedPtr<const FCategoryItem> Item;
private:

	/** Menu extender for right-click context menu */
	TSharedPtr<FExtender> MenuExtender;
	TSharedPtr<SWidget> BuildContextMenuContent(const TSharedPtr<FExtender>& InMenuExtender);
	void OnContextMenuClosed(TSharedRef<IMenu> Menu);
	/** Information about any active context menu widgets */
	FActiveTextEditContextMenu ActiveContextMenu;

	void SelectAllMessagesFromCategory();
	void IsolateCategory();
	void OpenColorPicker();

	FSlateColor HandleGetCategoryColor() const;

	bool IsSelected() const { return CategoryWidget->Filter.IsLogCategoryEnabled(Item->CategoryName); };
	const FSlateBrush* GetBorder() const;
	bool bIsPressed;

	const FSlateBrush* DeselectedImage;
	const FSlateBrush* HoveredImage;
	const FSlateBrush* HoveredSelectedImage;
	const FSlateBrush* SelectedImage;

	const FSlateBrush* HighlightBrush;
	

	SLogViewerProWidgetMain* MainWidget;
	SLogViewerProWidgetCategoriesView* CategoryWidget;
	

	bool FilterLogMessage(const TSharedRef<FLogMessage>& LogMessage);
	void ResetFilter();
};