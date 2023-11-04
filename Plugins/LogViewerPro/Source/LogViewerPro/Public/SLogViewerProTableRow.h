// Copyright 2019 Dmitry Labadin

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/SlateColor.h"
#include "Widgets/SWidget.h"
#include "Layout/Margin.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/STableRow.h"
//#include "SessionLogMessage.h"
#include "LogViewerProStructs.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Images/SImage.h"

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"

class Error;
class SLogViewerProWidgetMain;

/**
 * Implements a row widget for the session console log.
 */
class SLogViewerProTableRow
	: public SMultiColumnTableRow<TSharedPtr<FLogMessage>>
{
public:

	SLATE_BEGIN_ARGS(SLogViewerProTableRow) { }
		SLATE_ATTRIBUTE(FText, HighlightText)
		SLATE_ARGUMENT(TSharedPtr<FLogMessage>, LogMessage)
		SLATE_ARGUMENT(TSharedPtr<ISlateStyle>, Style)
		SLATE_ARGUMENT(SLogViewerProWidgetMain*, MainWidget)
		
		/** Menu extender for the right-click context menu */
		SLATE_EVENT(FMenuExtensionDelegate, ContextMenuExtender)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

public:

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:

	// SMultiColumnTableRow interface
		/** Menu extender for right-click context menu */
	TSharedPtr<FExtender> MenuExtender;
	TSharedPtr<SWidget> BuildContextMenuContent(const TSharedPtr<FExtender>& InMenuExtender);
	bool HasAnyLineSelected() const;
	void OnContextMenuClosed(TSharedRef<IMenu> Menu);
	
	/** Information about any active context menu widgets */
	FActiveTextEditContextMenu ActiveContextMenu;

	void CopySelectedRows();
	void SelectAllLogs();
	void DeleteSelected();
	void IsolateCategory();
	void DisableCategory();

	void OpenBPActor();
	void EditBlueprintClassByName(const FString& ClassName);
	void HihglightVector();






	/** Gets the border color for this row. */
	FSlateColor HandleGetBorderColor() const;

	/** Gets the text color for this log entry. */
	FSlateColor HandleGetTextColor() const;

	//Categories have different color logic than the main text block. It's too late in the evening to make that code more beautiful, but I will update it when I will streamline Error/Warnings colors
	FSlateColor HandleGetCategoryColor() const;

private:
	bool HasBPActorInSelectedLog() const;
	bool HasActorNamePatternInString(const FString& InputString) const;
	FString GetClassNameFromString(const FString& InputString) const;


	bool HasVectorInSelectedLog() const;
	bool HasVectorPatternInString(const FString& InputString) const;
	TArray<FVector> ExtractVectorsFromString(const FString& InputString);
	
	void HighlightVectorsInEditorWorld(const TArray<FVector>& InVectors);
	/** Holds the highlight string for the log message. */
	TAttribute<FText> HighlightText;

	/** Holds a reference to the log message that is displayed in this row. */
	TSharedPtr<FLogMessage> LogMessage;

	SLogViewerProWidgetMain* MainWidget;
};
