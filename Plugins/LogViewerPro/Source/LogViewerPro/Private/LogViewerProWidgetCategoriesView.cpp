// Copyright Dmitrii Labadin 2019

#include "LogViewerProWidgetCategoriesView.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SSearchBox.h"
#include "LogViewerProStructs.h"
#include "LogViewerProWidgetMain.h"
#include "LogViewerProSettingsButton.h"
#include "LogViewerPro.h"

#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"

#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "HAL/IPlatformFileModule.h"
#include "Framework/Commands/GenericCommands.h"


#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

namespace FLogViewerProCategoriesViewFilter
{
	void GetBasicStrings(const FCategoryItem& InCategoryItem, TArray<FString>& OutBasicStrings)
	{
		OutBasicStrings.Add(InCategoryItem.CategoryName.ToString());
	}
}

void SLogViewerProWidgetCategoriesView::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;
	Filter.OwnerWidget = MainWidget;

	SearchTextFilter = MakeShareable(new FCategoriesEntryTextFilter(
		FCategoriesEntryTextFilter::FItemToStringArray::CreateStatic(&FLogViewerProCategoriesViewFilter::GetBasicStrings)
	));

	const FButtonStyle& ButtonStyle = FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Menu.Button");

	DeselectedImage = ButtonStyle.Normal;
	HoveredImage = ButtonStyle.Pressed;
	HoveredSelectedImage = *FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder");
	HoveredSelectedImage.TintColor = FSlateColor(FLinearColor(0.f, 0.0f, 1.f));
	SelectedImage = *FAppStyle::Get().GetBrush("ToolPanel.GroupBorder");

	CategoryHighlightImage = FSlateBrush();

	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	ChildSlot
	[
		SNew(SBorder)
		.Padding(0)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				SAssignNew(SearchBoxPtr, SSearchBox)
				.HintText(NSLOCTEXT("LogViewerPro", "SearchCategories", "Search Categories (Alt + C)"))
				.OnTextChanged(this, &SLogViewerProWidgetCategoriesView::OnSearchChanged)
				.OnTextCommitted(this, &SLogViewerProWidgetCategoriesView::OnSearchCommitted)
			]

			+ SVerticalBox::Slot()
			.Padding(0)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::OnSaveCategoriesPressed)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.TextStyle(FAppStyle::Get(), "SmallText")
								.Text(LOCTEXT("Save", "Save"))
							]
						]
					]
				]
				
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::OnLoadCategoriesPressed)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.TextStyle(FAppStyle::Get(), "SmallText")
								.Text(LOCTEXT("Load", "Load"))
							]
						]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsDefaultCategoriesEnabled)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::OnMakeDefaultCategoriesPressed)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.TextStyle(FAppStyle::Get(), "SmallText")
								.Text_Lambda([&]() { return MainWidget->GetLogViewerProModule()->IsDefaultCategoriesEnabled() ? LOCTEXT("LogViewerProClearDefault", "Clear Default") : LOCTEXT("LogViewerProMakeDefault", "Make Default"); })
							]
						]
					]
				]
			]


			

			+ SVerticalBox::Slot()
			.Padding(0)
			[
				SAssignNew(DataDrivenContent, SBox)
				[
					SNew(SHorizontalBox)
				
					+ SHorizontalBox::Slot()
					[
						SAssignNew(ListView, SListView<TSharedPtr<FCategoryItem>>)
						.ListItemsSource(&Filter.FilteredItems)
						.OnGenerateRow(this, &SLogViewerProWidgetCategoriesView::OnGenerateWidgetForItem)
						.ExternalScrollbar(ScrollBar)
					]
				
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ScrollBar
					]
				]
			]

			+ SVerticalBox::Slot()
			.Padding(4)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteCategoriesEnableAll)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Enable All", "Enable All"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsCheckedVerbosityVeryVerbose)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteVerbosityVeryVerbose)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("VeryVerbose", "VeryVerbose"))
							]
						]
					]

					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsCheckedVerbosityVerbose)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteVerbosityVerbose)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Verbose", "Verbose"))
							]
						]
					]

					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsCheckedVerbosityLog)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteVerbosityLog)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Messages", "Messages"))
							]
						]
					]
				]
				
				+ SHorizontalBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked_Lambda([&]() { return ECheckBoxState::Unchecked; })
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteCategoriesDisableAll)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Disable All", "Disable All"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsCheckedVerbosityDisplay)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteVerbosityDisplay)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Display", "Display"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsCheckedVerbosityWarning)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteVerbosityWarning)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Warning", "Warnings"))
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(4)
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(this, &SLogViewerProWidgetCategoriesView::IsCheckedVerbosityError)
						.OnCheckStateChanged(this, &SLogViewerProWidgetCategoriesView::ExecuteVerbosityError)
						[
							SNew(SBox)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(FMargin(4.0, 2.0))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Errors", "Errors"))
							]
						]

					]
				]
			]
		]
	];
}

void SLogViewerProWidgetCategoriesView::Init()
{
	if (IsDefaultCategoriesFileExists())
	{
		MainWidget->GetLogViewerProModule()->DefaultCategoriesEnable();
		LoadDefaultCategories();
	}
	else
	{
		MainWidget->GetLogViewerProModule()->DefaultCategoriesDisable();
	}
}

TSharedRef<ITableRow> SLogViewerProWidgetCategoriesView::OnGenerateWidgetForItem(TSharedPtr<FCategoryItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FCategoryItem>>, OwnerTable)
		[
			SNew(SLogViewerProWidgetCategoriesEntry, InItem.ToSharedRef())
				.MainWidget(MainWidget)
				.CategoryWidget(this)
		];
}

bool SLogViewerProWidgetCategoriesView::IsSearchActive() const
{
	return !SearchTextFilter->GetRawFilterText().IsEmpty();
}

void SLogViewerProWidgetCategoriesView::ClearCategories()
{
	Filter.AllItems.Reset();
	Filter.FilteredItems.Reset();
	Filter.AvailableLogCategories.Reset();
	Filter.SelectedLogCategories.Reset();
}

void SLogViewerProWidgetCategoriesView::OnSearchChanged(const FText& InFilterText)
{
	MarkDirty();
	SearchTextFilter->SetRawFilterText(InFilterText);
	SearchBoxPtr->SetError(SearchTextFilter->GetFilterErrorText());
}

void SLogViewerProWidgetCategoriesView::OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	OnSearchChanged(InFilterText);
}

void SLogViewerProWidgetCategoriesView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bNeedsUpdate)
	{
		bNeedsUpdate = false;
		UpdateFilteredItems();
	}
}

void SLogViewerProWidgetCategoriesView::AddCategory(const FName& InCategory, bool bActivated)
{
	
	if (Filter.IsLogCategoryExist(InCategory))
	{
		return;
	}
	Filter.AddAvailableLogCategory(InCategory, bActivated);
	TSharedPtr<FCategoryItem> Item = MakeShareable(new FCategoryItem{ InCategory, true });
	Filter.AllItems.Add(Item);
	MainWidget->GetColorTable()->RegisterCategory(InCategory);
	MarkDirty();
}

void SLogViewerProWidgetCategoriesView::UpdateFilteredItems()
{
	if (IsSearchActive())
	{
		Filter.FilteredItems.Reset();
		auto FilterFtunction = [&](const TSharedPtr<FCategoryItem>& Item) { return SearchTextFilter->PassesFilter(*Item); };
		for (auto& Item : Filter.AllItems)
		{
			if (FilterFtunction(Item))
			{
				Filter.FilteredItems.Add(Item);
			}
		}
		Filter.FilteredItems.Sort(&FCategoryItem::SortItemsByName);
	}
	else
	{
		Filter.FilteredItems = Filter.AllItems;
		Filter.FilteredItems.Sort(&FCategoryItem::SortItemsByName);
	}

	//DataDrivenContent->SetVisibility(EVisibility::Visible);
	ListView->RequestListRefresh();
}

SLogViewerProWidgetCategoriesView::~SLogViewerProWidgetCategoriesView()
{

}

void SLogViewerProWidgetCategoriesEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FCategoryItem>& InItem)
{
	MainWidget = InArgs._MainWidget;
	CategoryWidget = InArgs._CategoryWidget;
	bIsPressed = CategoryWidget->Filter.IsLogCategoryEnabled(InItem->CategoryName);;
	Item = InItem;

	//const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");
	//const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("Menu.Button");
	
	DeselectedImage = &CategoryWidget->DeselectedImage;
	HoveredImage = &CategoryWidget->HoveredImage;
	SelectedImage = &CategoryWidget->SelectedImage;
	HoveredSelectedImage  = &CategoryWidget->HoveredSelectedImage;

	HighlightBrush = &CategoryWidget->CategoryHighlightImage;

	MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("EditCategory", EExtensionHook::Before, TSharedPtr<FUICommandList>(), InArgs._ContextMenuExtender);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(this, &SLogViewerProWidgetCategoriesEntry::GetBorder)
		//.Cursor(EMouseCursor::GrabHand)
		//.ToolTip(AssetEntryToolTip)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::AsCultureInvariant(InItem->CategoryName.ToString()))
				.ColorAndOpacity_Lambda([this]()
				{
					return this->HandleGetCategoryColor();
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f)
			.HAlign(EHorizontalAlignment::HAlign_Right)
			[
				SNew(SBorder)
				.BorderImage(HighlightBrush)
				.BorderBackgroundColor_Lambda([this]()
				{
						if (!this->IsSelected())
						{
							return FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
						}
						return this->HandleGetCategoryColor();
				})
				//.ForegroundColor_Lambda([this]()
				//{
				//	if (!this->IsSelected())
				//	{
				//		 return FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
				//	}
				//	return this->HandleGetCategoryColor();
				//})
				[
					SNew(SBox)
					.WidthOverride(1.0f)
					//.WidthOverride_Lambda([this]()
					//{
					//		return this->IsSelected() ? 1.f : 0.f;
					//		//return 0.f;
					//})
				]
			]
			
			//+ SVerticalBox::Slot()
			//.AutoHeight()
			//[
			//	SNew(SHorizontalBox)
			//	+ SHorizontalBox::Slot()
			//	.AutoWidth()
			//	[
			//		SNew(STextBlock)
			//		.Text(LOCTEXT("LogViewerCategories", "Color: "))
			//		.ColorAndOpacity(FSlateColor::UseForeground())
			//	]
			//	+ SHorizontalBox::Slot()
			//	.AutoWidth()
			//	[
			//		SNew(SColorBlock)
			//		.Color(HandleGetCategoryColor().GetSpecifiedColor())
			//		.Size(FVector2D(14, 14))
			//	]
			//]

			//+ SHorizontalBox::Slot()
			//.AutoWidth()
			//[
			//	SNew(SCheckBox)
			//]
		]
	];
}

TSharedPtr<SWidget> SLogViewerProWidgetCategoriesEntry::BuildContextMenuContent(const TSharedPtr<FExtender>& InMenuExtender)
{
	//FMenuBuilder MenuBuilder(true, NULL);
	FMenuBuilder MenuBuilder(true, NULL, InMenuExtender, true, &FCoreStyle::Get());

	//MenuBuilder.BeginSection("Category Options", LOCTEXT("LVCategoryOptions", "Category Options"));
	//{
		FUIAction SelectAllItemsAction(FExecuteAction::CreateSP(this, &SLogViewerProWidgetCategoriesEntry::SelectAllMessagesFromCategory));
		MenuBuilder.AddMenuEntry(LOCTEXT("LVChangeColor", "Select All Category Items"), LOCTEXT("LVChangeColorTooltip", "Select All Items of selected category"), FSlateIcon(), SelectAllItemsAction, NAME_None, EUserInterfaceActionType::Button);

		FUIAction IsolateCategoryAction(FExecuteAction::CreateSP(this, &SLogViewerProWidgetCategoriesEntry::IsolateCategory));
		MenuBuilder.AddMenuEntry(LOCTEXT("LVIsolateCategory", "Isolate Category"), LOCTEXT("LVChangeColorTooltip", "Disable all other cateogies, but not that one"), FSlateIcon(), IsolateCategoryAction, NAME_None, EUserInterfaceActionType::Button);
	
		FUIAction ColorPickerAction(FExecuteAction::CreateSP(this, &SLogViewerProWidgetCategoriesEntry::OpenColorPicker));
		MenuBuilder.AddMenuEntry(LOCTEXT("LVChangeColor", "Change Category Color"), LOCTEXT("LVChangeColorTooltip", "Customize color for selected category"), FSlateIcon(), ColorPickerAction, NAME_None, EUserInterfaceActionType::Button);

	//}

	return MenuBuilder.MakeWidget();
}

void SLogViewerProWidgetCategoriesEntry::OnContextMenuClosed(TSharedRef<IMenu> Menu)
{
	// Note: We don't reset the ActiveContextMenu here, as Slate hasn't yet finished processing window focus events, and we need 
	// to know that the window is still available for OnFocusReceived and OnFocusLost even though it's about to be destroyed

	// Give our owner widget focus when the context menu has been dismissed
	//if (CategoryWidget != nullptr)
	//{
	//	FSlateApplication::Get().SetKeyboardFocus(CategoryWidget, EFocusCause::OtherWidgetLostFocus);
	//}
}


FSlateColor SLogViewerProWidgetCategoriesEntry::HandleGetCategoryColor() const
{
	if(MainWidget->GetSettings()->IsShowLogColors())
	{ 
		return MainWidget->GetColorTable()->GetColorForCategory(Item->CategoryName);
	}
	else
	{
		return FSlateColor::UseForeground();
	}
}

FReply SLogViewerProWidgetCategoriesEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = FReply::Unhandled();

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//bIsPressed = true;
		ensure(MainWidget);
		MainWidget->CategoryMenu->Filter.ToggleLogCategory(Item->CategoryName);
		MainWidget->Refresh();

		Reply = FReply::Handled();
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
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
					ContextMenu->GetOnMenuDismissed().AddRaw(this, &SLogViewerProWidgetCategoriesEntry::OnContextMenuClosed);
					ActiveContextMenu.SummonSucceeded(ContextMenu.ToSharedRef());
				}
				else
				{
					ActiveContextMenu.SummonFailed();
				}
			}
		}

		// Release mouse capture
		Reply = FReply::Handled();
		Reply.ReleaseMouseCapture();

	}
	return Reply;
}

FReply SLogViewerProWidgetCategoriesEntry::ColorBlock_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	//OpenColorPicker();
	return FReply::Handled();
}

void SLogViewerProWidgetCategoriesEntry::SelectAllMessagesFromCategory()
{
		ensure(MainWidget);
		MainWidget->SelectAllByCategory(Item->CategoryName);
}

void SLogViewerProWidgetCategoriesEntry::IsolateCategory()
{
	MainWidget->CategoryMenu->ExecuteCategoriesDisableAll(ECheckBoxState::Undetermined);
	MainWidget->CategoryMenu->Filter.ToggleLogCategory(Item->CategoryName);
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesEntry::OpenColorPicker()
{
	TSharedPtr<SWindow> ColorWindow;
	SAssignNew(ColorWindow, SWindow)
		.Title(LOCTEXT("LVCategories", "Choose category color"))
		.ClientSize(SColorPicker::DEFAULT_WINDOW_SIZE)
		.IsPopupWindow(true);

	TSharedPtr<SBox> ColorPicker = SNew(SBox)
		.Padding(10.0f)
		[
			SNew(SColorPicker)
			.ParentWindow(ColorWindow)
			.UseAlpha(false)
			.OnlyRefreshOnOk(true)
			.TargetColorAttribute(this->MainWidget->GetColorTable()->GetColorForCategory(Item->CategoryName))
			.OnColorCommitted_Lambda([this](const FLinearColor& NewColor)
			{
				this->MainWidget->GetColorTable()->SetColorForCategory(Item->CategoryName, NewColor);
				this->MainWidget->Refresh();
				this->MainWidget->Invalidate(EInvalidateWidgetReason::Visibility);
				
				//FontColor = NewColor;
				//FontColor.A = 1.0f;
				//SampleTextBlock->SetColorAndOpacity(FontColor);
				;
			})
		];

	ColorWindow->SetContent(ColorPicker.ToSharedRef());

	FSlateApplication::Get().AddModalWindow(ColorWindow.ToSharedRef(), FGlobalTabmanager::Get()->GetRootWindow());
	//return FReply::Handled();
}

FReply SLogViewerProWidgetCategoriesEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}
	//if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	//{
	//	bIsPressed = false;
	//}
	//bIsPressed = false;
	return FReply::Handled();// .DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
}

bool SLogViewerProWidgetCategoriesEntry::IsPressed() const
{
	return bIsPressed;
}

const FSlateBrush* SLogViewerProWidgetCategoriesEntry::GetBorder() const
{
	if (IsHovered())
	{
		return IsSelected() ? HoveredSelectedImage : HoveredImage;
	}
	else if (IsSelected())
	{
		return SelectedImage;
	}
	else
	{
		return DeselectedImage;
	}
}

ECheckBoxState SLogViewerProWidgetCategoriesView::IsDefaultCategoriesEnabled() const
{
	return MainWidget->GetLogViewerProModule()->IsDefaultCategoriesEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
};

ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedVerbosityVeryVerbose() const { return Filter.bShowVeryVerbose ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedVerbosityVerbose() const { return Filter.bShowVerbose ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedVerbosityLog() const { return Filter.bShowLog ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedVerbosityDisplay() const { return Filter.bShowDisplay ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedVerbosityWarning() const { return Filter.bShowWarnings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedVerbosityError() const { return Filter.bShowErrors ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };
ECheckBoxState SLogViewerProWidgetCategoriesView::IsCheckedCategoriesShowAll() const { return Filter.bShowAllCategories ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };

ECheckBoxState SLogViewerProWidgetCategoriesView::CategoriesSingle_IsChecked(FName InName) const
{
	return Filter.IsLogCategoryEnabled(InName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SLogViewerProWidgetCategoriesView::ExecuteVerbosityVeryVerbose(ECheckBoxState CheckState)
{
	Filter.bShowVeryVerbose = !Filter.bShowVeryVerbose;
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::ExecuteVerbosityVerbose(ECheckBoxState CheckState)
{
	Filter.bShowVerbose = !Filter.bShowVerbose;
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::ExecuteVerbosityLog(ECheckBoxState CheckState)
{
	Filter.bShowLog = !Filter.bShowLog;
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::ExecuteVerbosityDisplay(ECheckBoxState CheckState)
{
	Filter.bShowDisplay = !Filter.bShowDisplay;
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::ExecuteVerbosityWarning(ECheckBoxState CheckState)
{
	Filter.bShowWarnings = !Filter.bShowWarnings;
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::ExecuteVerbosityError(ECheckBoxState CheckState)
{
	Filter.bShowErrors = !Filter.bShowErrors;
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::ExecuteCategoriesDisableAll(ECheckBoxState CheckState)
{
	Filter.ClearSelectedLogCategories();
	
	//@HACK - facing some memory issues when disabling categories directly, so just turn on and toggle after, sorry :) 
	for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
	{
		Filter.EnableLogCategory(AvailableCategory);
	}

	for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
	{
		Filter.ToggleLogCategory(AvailableCategory);
	}


	MainWidget->Refresh();
}


void SLogViewerProWidgetCategoriesView::ExecuteCategoriesEnableAll(ECheckBoxState CheckState)
{
	Filter.ClearSelectedLogCategories();

	for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
	{
		Filter.EnableLogCategory(AvailableCategory);
	}

	MainWidget->Refresh();
}

bool SLogViewerProWidgetCategoriesView::FilterLogMessage(const TSharedRef<FLogMessage>& LogMessage)
{
	// create or update category counter
	if (!Filter.IsMessageAllowed(LogMessage))
	{
		return false;
	}
	
	return true;


	//if (HighlightOnlyCheckBox->IsChecked() || FilterStringTextBox->GetText().IsEmpty())
	//{
	//	return true;
	//}
	//
	//return (LogMessage->Message.Contains(FilterStringTextBox->GetText().ToString()));
}

void SLogViewerProWidgetCategoriesView::OnLoadCategoriesPressed(ECheckBoxState CheckState)
{
	LoadSelectedCategories();
}

void SLogViewerProWidgetCategoriesView::OnSaveCategoriesPressed(ECheckBoxState CheckState)
{
	SaveSelectedCategories();
}

void SLogViewerProWidgetCategoriesView::OnMakeDefaultCategoriesPressed(ECheckBoxState CheckState)
{
	if (!IsDefaultCategoriesFileExists())
	{
		MainWidget->GetLogViewerProModule()->DefaultCategoriesEnable();
		SaveDefaultCategorties();
	}
	else
	{
		MainWidget->GetLogViewerProModule()->DefaultCategoriesDisable();
		ClearDefaultCategories();
	}
}

void SLogViewerProWidgetCategoriesView::ClearDefaultCategories()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "Saving is not supported on this platform!"));
		return;
	}

	if (!IsDefaultCategoriesFileExists())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CLearDefaultCategoriesDialogUnsupportedError", "Trying to clear Default categories, but file not found"));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteFile(*GetDefaultCategoriesFilePath());
}

bool SLogViewerProWidgetCategoriesView::IsDefaultCategoriesFileExists() const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.FileExists(*GetDefaultCategoriesFilePath());
}

FString SLogViewerProWidgetCategoriesView::GetLogViewerConfigPath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewerPro");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}

FString SLogViewerProWidgetCategoriesView::GetDefaultCategoriesFilePath() const
{
	FString Path = FPaths::ProjectConfigDir() + TEXT("LogViewerPro") + TEXT("/DefaultCategories.ini");
	FPaths::NormalizeDirectoryName(Path);
	return Path;
}

void SLogViewerProWidgetCategoriesView::SaveDefaultCategorties()
{
	if(!CreateDefaultConfigFolder())
	{
		return;
	}
	FString Filename = GetDefaultCategoriesFilePath();
	SaveCategoriesIntoFile(Filename);
}

void SLogViewerProWidgetCategoriesView::SaveSelectedCategories()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "Saving is not supported on this platform!"));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//manage previously open folder
	if (!LastCategoriesFileSaveDirectory.IsEmpty())
	{
		if (!PlatformFile.DirectoryExists(*LastCategoriesFileSaveDirectory))
		{
			LastCategoriesFileSaveDirectory.Empty();
		}
	}

	//If no previously open folder, make default folder
	if (LastCategoriesFileSaveDirectory.IsEmpty())
	{
		CreateDefaultConfigFolder();
		LastCategoriesFileSaveDirectory = GetLogViewerConfigPath();
	}

	TArray<FString> Filenames;

	// open file dialog
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	void* ParentWindowHandle = (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid()) ? ParentWindow->GetNativeWindow()->GetOSWindowHandle() : nullptr;

	if (!DesktopPlatform->SaveFileDialog(
		ParentWindowHandle,
		LOCTEXT("SaveLogDialogTitle", "Save Categories As...").ToString(),
		LastCategoriesFileSaveDirectory,
		FString::Printf(TEXT("LogViewerProCategories-%s.ini"), *FDateTime::Now().ToString()),
		//TEXT("LogViewerProOutput-", FDateTime::Now().ToString(), ".log"),
		TEXT("Ini Files (*.ini)|*.ini"),
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
	LastCategoriesFileSaveDirectory = FPaths::GetPath(Filename);

	// add a file extension if none was provided
	if (FPaths::GetExtension(Filename).IsEmpty())
	{
		Filename += Filename + TEXT(".ini");
	}

	SaveCategoriesIntoFile(Filename);
}

void SLogViewerProWidgetCategoriesView::SaveCategoriesIntoFile(const FString& Filename)
{
	FArchive* LogFile = IFileManager::Get().CreateFileWriter(*Filename);
	if (LogFile == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogFileError", "Failed to open the specified file for saving!"));
		return;
	}

	FLVCategoriesSave CategoriesSave;
	CategoriesSave.Categories.Reserve(Filter.AvailableLogCategories.Num());

	for (const auto& Category : Filter.AvailableLogCategories)
	{
		FLVCategoriesSaveItem Item = FLVCategoriesSaveItem(Category.ToString(), Filter.IsLogCategoryEnabled(Category), MainWidget->GetColorTable()->GetColorForCategory(Category).ToFColor(true).ToHex());
		CategoriesSave.Categories.Add(Item);
	}

	const FString SettingsString = CategoriesSave.ToJson(true);
	LogFile->Serialize(TCHAR_TO_ANSI(*SettingsString), SettingsString.Len());
	LogFile->Close();
	delete LogFile;
}

bool SLogViewerProWidgetCategoriesView::CreateDefaultConfigFolder()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (DesktopPlatform == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogUnsupportedError", "File managemetn is not supported on selected platform!"));
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString DefaultPath = GetLogViewerConfigPath();
	FPaths::NormalizeDirectoryName(DefaultPath);
	if (!PlatformFile.DirectoryExists(*DefaultPath))
	{
		PlatformFile.CreateDirectory(*DefaultPath);
	}
	return true;
}


//LOAD CATEGORIES
void SLogViewerProWidgetCategoriesView::LoadDefaultCategories()
{
	FString Path = GetDefaultCategoriesFilePath();

	FLVCategoriesSave CategoriesSave;
	const bool bParsed = ParseCategoriesFile(Path, CategoriesSave);
	if (!bParsed)
	{
		UE_LOG(LogTemp, Error, TEXT("LogViewer: Unable to parse categories save file using new formart. Previous JSON warning in logs is related to that issue. Trying old format..."));
		const bool bLegacyParse = ParseCategoriesFileOld(Path, CategoriesSave);
		if (!bLegacyParse)
		{
			UE_LOG(LogTemp, Error, TEXT("LogViewer: Unable to parse categories save file using old format."));
			return;
		}
	}
	ApplyCategoriesSave(CategoriesSave);
	MainWidget->Refresh();
}

void SLogViewerProWidgetCategoriesView::LoadSelectedCategories()
{

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!LastCategoriesFileSaveDirectory.IsEmpty())
	{
		if (!PlatformFile.DirectoryExists(*LastCategoriesFileSaveDirectory))
		{
			LastCategoriesFileSaveDirectory.Empty();
		}
	}

	if (LastCategoriesFileSaveDirectory.IsEmpty())
	{
		CreateDefaultConfigFolder();
		LastCategoriesFileSaveDirectory = GetLogViewerConfigPath();
	}

	const FString ProjectFileDescription = LOCTEXT("Categories Description", "Categories files").ToString();
	const FString LogFileType(TEXT("ini"));
	const FString LogFileExtension = FString::Printf(TEXT("*.%s"), *LogFileType);
	const FString FileTypes = FString::Printf(TEXT("%s (%s)|%s"), *ProjectFileDescription, *LogFileExtension, *LogFileExtension);

	// Prompt the user for the filenames
	TArray<FString> OpenFilenames;

	bool bOpened = false;
	void* ParentWindowWindowHandle = NULL;

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
	if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
	{
		ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return;
	}

	bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowWindowHandle,
		LOCTEXT("OpenCategories", "Open Saved Categories").ToString(),
		LastCategoriesFileSaveDirectory,
		TEXT(""),
		FileTypes,
		EFileDialogFlags::None,
		OpenFilenames
	);

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
	LastCategoriesFileSaveDirectory = FPaths::GetPath(Path);


	FLVCategoriesSave CategoriesSave;
	if (!ParseCategoriesFile(Path, CategoriesSave))
	{
		if (!ParseCategoriesFileOld(Path, CategoriesSave))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLogDialogFileError", "Unable to parse default categories file. Something is wrong, maybe file format!"));
			return;
		}
	}
	ApplyCategoriesSave(CategoriesSave);

	MainWidget->Refresh();
}

bool SLogViewerProWidgetCategoriesView::ParseCategoriesFile(const FString& Path, FLVCategoriesSave& CategoriesSave)
{
	if (!FPaths::FileExists(Path))
	{
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Path))
	{
		return false;
	}

	const bool bParsingSuccessful = CategoriesSave.FromJson(JsonString);

	if (!bParsingSuccessful)
	{
		return false;
	}

	if (CategoriesSave.Categories.Num() <= 0)
	{
		return false;
	}

	return true;
}

bool SLogViewerProWidgetCategoriesView::ParseCategoriesFileOld(const FString& Path, FLVCategoriesSave& CategoriesSave)
{
	TArray<FString> CategoriesStrings;
	if (!FFileHelper::LoadANSITextFileToStrings(*Path, nullptr, CategoriesStrings) == true)
	{
		return false;
	}

	if (CategoriesStrings.Num() == 0)
	{
		return false;
	}

	for (const auto& CategoryString : CategoriesStrings)
	{
		FString Category;
		FString State;
		bool bResult = CategoryString.Split(TEXT(" = "), &Category, &State);
		if (!bResult)
		{
			continue;
		}

		bool bisActive = State.Equals(TEXT("Off"));
		FLVCategoriesSaveItem NewItem(Category, bisActive, FLogColorTable::GenerateRandomColor().ToFColor(true).ToHex());
		CategoriesSave.Categories.Add(NewItem);
	}
	return true;
}

void SLogViewerProWidgetCategoriesView::ApplyCategoriesSave(const FLVCategoriesSave& CategoriesSave)
{
	for (const auto& CategoryItem : CategoriesSave.Categories)
	{
		const FName CategoryName = FName(CategoryItem.CategoryName);

		if (!Filter.IsLogCategoryExist(CategoryName))
		{
			AddCategory(CategoryName, true);
		}
		Filter.EnableLogCategory(CategoryName);
		if (!CategoryItem.bIsActive)
		{
			Filter.ToggleLogCategory(CategoryName);
		}

		FColor Color = FColor::FromHex(CategoryItem.CategoryHexColor);
		MainWidget->GetColorTable()->SetColorForCategory(CategoryName, Color);
	}
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE