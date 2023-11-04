
#include "SLogViewerProTableRow.h"

#include "LogViewerProWidgetMain.h"
#include "LogViewerProWidgetCategoriesView.h"
#include "LogViewerPro.h"
#include "LogViewerProSettingsButton.h"

#include "Framework/Commands/GenericCommands.h"
#include "Internationalization/Regex.h"

#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerProTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	HighlightText = InArgs._HighlightText;
	LogMessage = InArgs._LogMessage;
	MainWidget = InArgs._MainWidget;

	//If you know how to change the color - email me please dimalabad@gmail.com
	//FTableRowStyle* TableRawStyle  = new FTableRowStyle();
	//FSlateBrush Brush;
	//Brush.TintColor = FLinearColor::Gray;
	//TableRawStyle->ActiveBrush = Brush;
	//Style = TableRawStyle;
	//check(InArgs._Style.IsValid());
	//Style = InArgs._Style;

	MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("EditCategory", EExtensionHook::Before, TSharedPtr<FUICommandList>(), InArgs._ContextMenuExtender);

	SMultiColumnTableRow<TSharedPtr<FLogMessage>>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}



TSharedRef<SWidget> SLogViewerProTableRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	const bool bIsError = (LogMessage->Verbosity == ELogVerbosity::Error) || (LogMessage->Verbosity == ELogVerbosity::Fatal);
	const bool bIsWarning = (LogMessage->Verbosity == ELogVerbosity::Warning);

	const FLinearColor ColorWarning = FLinearColor(0.6f, 0.6f, 0.0f);
	const FLinearColor ColorError = FLinearColor(0.3f, 0.05f, 0.05f);
	const FVector2D MsgShadowOffset = FVector2D(0.5f, 0.5f);



	TSharedPtr< STextBlock > MessageTextBlock = SNew(STextBlock)
		.ColorAndOpacity_Lambda([this]()
			{
				return this->HandleGetTextColor();
			})
		.HighlightText(HighlightText)
		//.Text(FText::FromString(LogMessage->Message->Replace(TEXT("\n"), TEXT(" | ")).Replace(TEXT("\r"), TEXT(""))));
		//.Text(FText::FromString(LogMessage->Message.Get()));
		.Text_Lambda([this]()
			{
				const FText LogMessageText = MainWidget->GetSettings()->IsCompactRow() ?
						FText::FromString(LogMessage->Message->Replace(TEXT("\n"), TEXT(" | ")).Replace(TEXT("\r"), TEXT(""))) :
						FText::FromString(LogMessage->Message.Get());

				return LogMessageText;
			});

			const FSlateColor CategoryColor = MainWidget->GetSettings()->IsShowLogColors() ? MainWidget->GetColorTable()->GetColorForCategory(LogMessage->Category) : FSlateColor::UseForeground();

			if (ColumnName == "Verbosity")
			{
				const FSlateBrush* Icon;



				if (bIsError)
				{
					Icon = FAppStyle::Get().GetBrush("Icons.Error");

					MessageTextBlock->SetShadowOffset(MsgShadowOffset);
					MessageTextBlock->SetShadowColorAndOpacity(ColorError);
				}
				else if (bIsWarning)
				{
					Icon = FAppStyle::Get().GetBrush("Icons.Warning");
					MessageTextBlock->SetShadowOffset(MsgShadowOffset);
					MessageTextBlock->SetShadowColorAndOpacity(ColorWarning);
				}
				else
				{
					Icon = FAppStyle::Get().GetBrush("Icons.Info");
				}

				return SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					[
						SNew(SImage)
						.Image(Icon)
					];
			}
			if (ColumnName == "Time")
			{
				static const FNumberFormattingOptions FormatOptions = FNumberFormattingOptions()
					.SetMinimumFractionalDigits(3)
					.SetMaximumFractionalDigits(3);

				return SNew(SBox)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity_Lambda([this]()
							{
								return this->HandleGetTextColor();
							})
					.Text(LogMessage->bNoTime ? FText() : FText::FromString(LogMessage->GetTimeHoursString()))
					];
			}
			else if (ColumnName == "TimeSeconds")
			{
				static const FNumberFormattingOptions FormatOptions = FNumberFormattingOptions()
					.SetMinimumFractionalDigits(3)
					.SetMaximumFractionalDigits(3)
					.SetUseGrouping(false);

				return SNew(SBox)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity_Lambda([this]()
							{
								return this->HandleGetTextColor();
							})
					.Text(LogMessage->bNoTime ? FText() : FText::AsNumber(LogMessage->TimeSeconds, &FormatOptions))
					];
			}
			else if (ColumnName == "Frame")
			{
				return SNew(SBox)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity_Lambda([this]()
							{
								return this->HandleGetTextColor();
							})
					.Text(LogMessage->bNoTime ? FText() : FText::AsNumber(LogMessage->FrameNumber % 1000))
					];
			}
			else if (ColumnName == "Category")
			{
				return SNew(SBox)
					.Padding(FMargin(4.0f, 0.0f))
					.VAlign(VAlign_Top)
					[
						SNew(STextBlock)
						.ColorAndOpacity_Lambda([this]()
							{
								return this->HandleGetCategoryColor();
							})
					.HighlightText(HighlightText)
								.Text(FText::FromName(LogMessage->Category))
								//.Font(FEditorStyle::GetFontStyle("BoldFont"))
					];
			}
			else if (ColumnName == "Message")
			{
				return SNew(SBox)
					.Padding(FMargin(4.0f, 0.0f))
					.VAlign(VAlign_Top)
					[
						MessageTextBlock.ToSharedRef()
						//SNew(SBorder)
						//	.BorderImage(FAppStyle::GetBrush("Menu.Background"))
						//	.BorderBackgroundColor(ColorWarningBackground)
						//	.ForegroundColor(ColorWarningBackground)
						//[
						//
						//	SNew(STextBlock)
						//		.ColorAndOpacity(HandleGetTextColor())
						//		.HighlightText(HighlightText)
						//		.ShadowColorAndOpacity(ColorWarningBackground)
						//		.ShadowOffset(FVector2D(0.5f, 0.5f))
						//		.Text(FText::FromString(LogMessage->Message->Replace(TEXT("\n"), TEXT(" | ")).Replace(TEXT("\r"), TEXT(""))))
						//]
					];
			}

			return SNullWidget::NullWidget;
}

FReply SLogViewerProTableRow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = FReply::Unhandled();

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Reply = SMultiColumnTableRow::OnMouseButtonDown(MyGeometry, InMouseEvent);
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		Reply = SMultiColumnTableRow::OnMouseButtonDown(MyGeometry, InMouseEvent);
		if (MyGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			// Right clicked, so summon a context menu if the cursor is within the widget
			FWidgetPath WidgetPath = InMouseEvent.GetEventPath() != nullptr ? *InMouseEvent.GetEventPath() : FWidgetPath();

			TSharedPtr<SWidget> MenuContentWidget = BuildContextMenuContent(MenuExtender);
			if (MenuContentWidget.IsValid())
			{
				ActiveContextMenu.PrepareToSummon();

				static const bool bFocusImmediately = true;
				TSharedPtr<IMenu> ContextMenu = FSlateApplication::Get().PushMenu(
					InMouseEvent.GetWindow(),
					WidgetPath,
					MenuContentWidget.ToSharedRef(),
					InMouseEvent.GetScreenSpacePosition(),
					FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu),
					bFocusImmediately
				);

				// Make sure the window is valid. It's possible for the parent to already be in the destroy queue, for example if the editable text was configured to dismiss it's window during OnTextCommitted.
				if (ContextMenu.IsValid())
				{
					ContextMenu->GetOnMenuDismissed().AddRaw(this, &SLogViewerProTableRow::OnContextMenuClosed);
					ActiveContextMenu.SummonSucceeded(ContextMenu.ToSharedRef());
				}
				else
				{
					ActiveContextMenu.SummonFailed();
				}
			}
		}

		// Release mouse capture
		//Reply = FReply::Handled();
		//Reply.ReleaseMouseCapture();

	}
	return Reply;
}


TSharedPtr<SWidget> SLogViewerProTableRow::BuildContextMenuContent(const TSharedPtr<FExtender>& InMenuExtender)
{
	//FMenuBuilder MenuBuilder(true, NULL);
	FMenuBuilder MenuBuilder(true, NULL, InMenuExtender, true, &FCoreStyle::Get());

	MenuBuilder.BeginSection("Log Options", LOCTEXT("LVCategoryOptions", "Edit"));
	{
		FUIAction CopyRowsAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::CopySelectedRows),
			FCanExecuteAction::CreateSP(this, &SLogViewerProTableRow::HasAnyLineSelected));
		MenuBuilder.AddMenuEntry(LOCTEXT("LvCopy", "Copy (Ctrl+C)"), LOCTEXT("LVCopytooltip", "Copy selected items"), FSlateIcon(), CopyRowsAction, NAME_None, EUserInterfaceActionType::Button);	

		FUIAction DeleteSelectedAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::DeleteSelected),
			FCanExecuteAction::CreateSP(this, &SLogViewerProTableRow::HasAnyLineSelected));
		MenuBuilder.AddMenuEntry(LOCTEXT("LvDeleteSelected", "Delete (Del)"), LOCTEXT("LvDeleteSelectedtooltip", "Delete Selected Items"), FSlateIcon(), DeleteSelectedAction, NAME_None, EUserInterfaceActionType::Button);

		FUIAction SelectAllAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::SelectAllLogs));
		MenuBuilder.AddMenuEntry(LOCTEXT("LvSelectAll", "Select All (Ctrl+A)"), LOCTEXT("LvSelectAlltooltip", "Select all visible items"), FSlateIcon(), SelectAllAction, NAME_None, EUserInterfaceActionType::Button);

	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("Log Options", LOCTEXT("LVCategoryOptions", "Actions"));
	{
		FUIAction DisableategoryAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::DisableCategory),
			FCanExecuteAction::CreateSP(this, &SLogViewerProTableRow::HasAnyLineSelected));
		MenuBuilder.AddMenuEntry(LOCTEXT("LvDisable", "Disable Category"), LOCTEXT("LVDisableTooltip", "Disable categories for selected items"), FSlateIcon(), DisableategoryAction, NAME_None, EUserInterfaceActionType::Button);

		FUIAction IsolateCategoryAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::IsolateCategory),
			FCanExecuteAction::CreateSP(this, &SLogViewerProTableRow::HasAnyLineSelected));
		MenuBuilder.AddMenuEntry(LOCTEXT("LvCopy", "Isolate Category"), LOCTEXT("LVIsloateTooltip", "Activate categories for selected items and disable the rest"), FSlateIcon(), IsolateCategoryAction, NAME_None, EUserInterfaceActionType::Button);

		FUIAction OpenBPActorAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::OpenBPActor), 
			FCanExecuteAction::CreateSP(this, &SLogViewerProTableRow::HasBPActorInSelectedLog));

		MenuBuilder.AddMenuEntry(LOCTEXT("LVChangeColor", "Edit Actor BP"), LOCTEXT("LVChangeColorTooltip", "Edit first discovered Blueprint Class from the selected log."), FSlateIcon(), OpenBPActorAction, NAME_None, EUserInterfaceActionType::Button);
		//
		FUIAction HighlightVectorAction(FExecuteAction::CreateSP(this, &SLogViewerProTableRow::HihglightVector),
			FCanExecuteAction::CreateSP(this, &SLogViewerProTableRow::HasVectorInSelectedLog));

		MenuBuilder.AddMenuEntry(LOCTEXT("LVChangeColor", "Highlight Vectors"), LOCTEXT("LVHighlightVectorTooltip", "Highlight vectors from selected log message in editor viewport"), FSlateIcon(), HighlightVectorAction, NAME_None, EUserInterfaceActionType::Button);
	}

	return MenuBuilder.MakeWidget();
}

bool SLogViewerProTableRow::HasAnyLineSelected() const
{
	const int32 SelectedItemCount = MainWidget->LogListView->GetNumItemsSelected();
	return SelectedItemCount > 0 ? true : false;
}
bool SLogViewerProTableRow::HasBPActorInSelectedLog() const
{
	const int32 SelectedItemCount = MainWidget->LogListView->GetNumItemsSelected();
	if (SelectedItemCount > 50) //Make hard limit when you don't process
	{
		return false;
	}

	TArray<TSharedPtr<FLogMessage>> SelectedItems = MainWidget->LogListView->GetSelectedItems();

	for (auto& Item : SelectedItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		if (HasActorNamePatternInString(Item->Message.Get()))
		{
			return true;
		}
	}
	return false;
}

bool SLogViewerProTableRow::HasActorNamePatternInString(const FString& InputString) const
{
	FString Pattern = TEXT("BP_([A-Za-z0-9]+)[0-9]*");

	FRegexPattern RegexPattern(Pattern);
	FRegexMatcher Matcher(RegexPattern, InputString);

	if (Matcher.FindNext())
	{
		FString ActorName = Matcher.GetCaptureGroup(1);
		return true;
	}

	return false;
}

FString SLogViewerProTableRow::GetClassNameFromString(const FString& InputString) const
{
	FString Pattern = TEXT("(BP_[A-Za-z0-9]+)[0-9]*");

	FRegexPattern RegexPattern(Pattern);
	FRegexMatcher Matcher(RegexPattern, InputString);

	if (Matcher.FindNext())
	{
		FString ActorName = Matcher.GetCaptureGroup(1);
		return ActorName;
	}

	return FString();
}

void SLogViewerProTableRow::OpenBPActor()
{
	TArray<TSharedPtr<FLogMessage>> SelectedItems = MainWidget->LogListView->GetSelectedItems();

	for (auto& Item : SelectedItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		if (HasActorNamePatternInString(Item->Message.Get()))
		{
			FString ActorName = GetClassNameFromString(Item->Message.Get());
			EditBlueprintClassByName(ActorName);
			break;
		}
	}
}

void SLogViewerProTableRow::EditBlueprintClassByName(const FString& ClassName)
{
	UBlueprint* BlueprintToEdit = nullptr;
	for (TObjectIterator<UBlueprint> It; It; ++It)
	{
		UBlueprint* Blueprint = *It;

		FString BPClassName = Blueprint->GetName();
		if (BPClassName.Equals(ClassName, ESearchCase::IgnoreCase))
		{
			BlueprintToEdit = Blueprint;
			break; 
		}
	}

	GEditor->EditObject(BlueprintToEdit);
}

bool SLogViewerProTableRow::HasVectorInSelectedLog() const
{
	const int32 SelectedItemCount = MainWidget->LogListView->GetNumItemsSelected();
	if (SelectedItemCount > 50) //Make hard limit when you don't process
	{
		return false;
	}
	
	TArray<TSharedPtr<FLogMessage>> SelectedItems = MainWidget->LogListView->GetSelectedItems();

	for (auto& Item : SelectedItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		if (HasVectorPatternInString(Item->Message.Get()))
		{
			return true;
		}
	}
	return false;
}

bool SLogViewerProTableRow::HasVectorPatternInString(const FString& InputString) const
{
	// Define the regular expression pattern for matching the vector pattern
	FString Pattern = TEXT("X=(-?\\d+\\.?\\d*)\\s+Y=(-?\\d+\\.?\\d*)\\s+Z=(-?\\d+\\.?\\d*)");

	// Create a regular expression instance
	FRegexPattern RegexPattern(Pattern);
	FRegexMatcher Matcher(RegexPattern, InputString);

	// Perform the matching
	if (Matcher.FindNext())
	{
		// Extract the matched values
		FString XValue = Matcher.GetCaptureGroup(1);
		FString YValue = Matcher.GetCaptureGroup(2);
		FString ZValue = Matcher.GetCaptureGroup(3);

		// You can now use X, Y, and Z in your logic if necessary
		return true;
	}

	return false;
}

void SLogViewerProTableRow::HihglightVector()
{
	TArray<TSharedPtr<FLogMessage>> SelectedItems = MainWidget->LogListView->GetSelectedItems();

	TArray<FVector> VectorsToHighlight;
	for (auto& Item : SelectedItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		VectorsToHighlight.Append(ExtractVectorsFromString(Item->Message.Get()));
	}

	HighlightVectorsInEditorWorld(VectorsToHighlight);
}

TArray<FVector> SLogViewerProTableRow::ExtractVectorsFromString(const FString& InputString)
{
	FString Pattern = TEXT("X=(-?\\d+\\.?\\d*)\\s+Y=(-?\\d+\\.?\\d*)\\s+Z=(-?\\d+\\.?\\d*)");

	FRegexPattern RegexPattern(Pattern);
	FRegexMatcher Matcher(RegexPattern, InputString);

	TArray<FVector> OutVectors;

	while (Matcher.FindNext())
	{
		FString XValue = Matcher.GetCaptureGroup(1);
		FString YValue = Matcher.GetCaptureGroup(2);
		FString ZValue = Matcher.GetCaptureGroup(3);

		FVector Vector;
		Vector.X = FCString::Atof(*XValue);
		Vector.Y = FCString::Atof(*YValue);
		Vector.Z = FCString::Atof(*ZValue);

		OutVectors.Add(Vector);
	}

	return OutVectors;
}

void SLogViewerProTableRow::HighlightVectorsInEditorWorld(const TArray<FVector>& InVectors)
{
	FWorldContext* WorldContext = GEditor->GetPIEWorldContext();
	if (!WorldContext)
		WorldContext = &GEditor->GetEditorWorldContext(false);
	if (!WorldContext)
		return;
	UWorld* CurrentWorld = WorldContext->World();
	
	const float DrawTime = 5.f;

	for( const FVector& Vector : InVectors)
	{
		DrawDebugPoint(CurrentWorld, Vector, 20.f, FColor(16, 16, 16), false, DrawTime, MAX_uint8);
		DrawDebugPoint(CurrentWorld, Vector, 12.f, FColor(255, 182, 0), false, DrawTime-1.f, MAX_uint8);
		DrawDebugPoint(CurrentWorld, Vector, 8.f, FColor(16, 16, 0), false, DrawTime - 1.f, MAX_uint8);
	}

}

void SLogViewerProTableRow::OnContextMenuClosed(TSharedRef<IMenu> Menu)
{
	// Note: We don't reset the ActiveContextMenu here, as Slate hasn't yet finished processing window focus events, and we need 
	// to know that the window is still available for OnFocusReceived and OnFocusLost even though it's about to be destroyed

	// Give our owner widget focus when the context menu has been dismissed
	//if (CategoryWidget != nullptr)
	//{
	//	FSlateApplication::Get().SetKeyboardFocus(CategoryWidget, EFocusCause::OtherWidgetLostFocus);
	//}
}

void SLogViewerProTableRow::CopySelectedRows()
{
	MainWidget->CopySelectedLogs();
}

void SLogViewerProTableRow::SelectAllLogs()
{
	MainWidget->SelectAllActiveLogs();
}

void SLogViewerProTableRow::DeleteSelected()
{
	MainWidget->DeleteSelectedMessages();
}

void SLogViewerProTableRow::IsolateCategory()
{
	TArray<FName> CategoriesToIsolate;
	for (const auto& Item : MainWidget->LogListView->GetSelectedItems())
	{
		CategoriesToIsolate.AddUnique(Item->Category);
	}

	MainWidget->CategoryMenu->ExecuteCategoriesDisableAll(ECheckBoxState::Undetermined);
	for (const FName& CategoryName : CategoriesToIsolate)
	{
		//@TODO make more explicit Toggle Category, for now called only in two places, so direct call is okay
		MainWidget->CategoryMenu->Filter.ToggleLogCategory(CategoryName);
	}
	MainWidget->Refresh();
}

void SLogViewerProTableRow::DisableCategory()
{
	TSet<FName> CategoriesToDisable;
	for (const auto& Item : MainWidget->LogListView->GetSelectedItems())
	{
		if (CategoriesToDisable.Contains(Item->Category))
		{
			continue;
		}

		CategoriesToDisable.Add(Item->Category);
		MainWidget->CategoryMenu->Filter.ToggleLogCategory(Item->Category);
	}
	MainWidget->Refresh();
}




FSlateColor SLogViewerProTableRow::HandleGetBorderColor() const
{
	//return FLinearColor((GetTypeHash(LogMessage->InstanceId) & 0xff) * 360.0f / 256.0f, 0.8f, 0.3f, 1.0f).HSVToLinearRGB();
	return FLinearColor(/*(GetTypeHash(LogMessage->InstanceId) & 0xff) * */360.0f / 256.0f, 0.8f, 0.3f, 1.0f).HSVToLinearRGB();
}

FSlateColor SLogViewerProTableRow::HandleGetTextColor() const
{
	//return FLinearColor(FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f));
	if ((LogMessage->Verbosity == ELogVerbosity::Error) ||
		(LogMessage->Verbosity == ELogVerbosity::Fatal))
	{
		return FLinearColor::Red;
	}
	else if (LogMessage->Verbosity == ELogVerbosity::Warning)
	{
		return FLinearColor::Yellow;
	}
	else
	{
		if (MainWidget->GetSettings()->IsShowLogColors())
		{
			return MainWidget->GetColorTable()->GetColorForCategory(LogMessage->Category);
		}
		else
		{
			return FSlateColor::UseForeground();
		}
	}
	//#endif
}

FSlateColor SLogViewerProTableRow::HandleGetCategoryColor() const
{
	if (MainWidget->GetSettings()->IsShowLogColors())
	{
		return MainWidget->GetColorTable()->GetColorForCategory(LogMessage->Category);
	}
	else
	{
		if ((LogMessage->Verbosity == ELogVerbosity::Error) ||
			(LogMessage->Verbosity == ELogVerbosity::Fatal))
		{
			return FLinearColor::Red;
		}
		else if (LogMessage->Verbosity == ELogVerbosity::Warning)
		{
			return FLinearColor::Yellow;
		}
		else
		{
			return FSlateColor::UseForeground();
		}
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
