// Copyright Dmitrii Labadin 2019

#include "LogViewerProWidgetMinimap.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SSearchBox.h"
#include "LogViewerProStructs.h"
#include "LogViewerProWidgetMain.h"
#include "LogViewerProWidgetTopBar.h"
#include "LogViewerProSettingsButton.h"
#include "Misc/PackageName.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "SLogViewerProWidgetMain"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogViewerProWidgetMinimap::Construct(const FArguments& InArgs)
{
	MainWidget = InArgs._MainWidget;

	//SearchTextFilter = MakeShareable(new FCategoriesEntryTextFilter(
	//	FCategoriesEntryTextFilter::FItemToStringArray::CreateStatic(&FLogViewerProCategoriesViewFilter::GetBasicStrings)
	//));

	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(&Brush)	
	];

	CreateTexture();
	UpdateTexture(0);
	RequestUpdate();
}

SLogViewerProWidgetMinimap::~SLogViewerProWidgetMinimap()
{
	if (NewTexture)
	{
		NewTexture->RemoveFromRoot();
	}
}

void SLogViewerProWidgetMinimap::CreateTexture()
{
	NewTexture = Cast<UTexture2D>(Brush.GetResourceObject());

	if (!NewTexture)
	{
		NewTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_B8G8R8A8);
		if (!NewTexture)
		{
			return;
		}
		NewTexture->MipGenSettings = TMGS_NoMipmaps;
		NewTexture->NeverStream = true;
		NewTexture->AddToRoot();

		Brush.SetResourceObject(NewTexture);
		Brush.ImageSize = FDeprecateSlateVector2D((float)TextureWidth, (float)TextureHeight);
		Brush.DrawAs = ESlateBrushDrawType::Image;

		FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
		uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
		for (uint32 y = 0; y < TextureHeight; y++)
		{
			for (uint32 x = 0; x < TextureWidth; x++)
			{
				int32 CurrentPixelIndex = ((y * TextureWidth) + x);
				Pixels[4 * CurrentPixelIndex + 2] = BgColor.R; //r
				Pixels[4 * CurrentPixelIndex + 1] = BgColor.G; //g
				Pixels[4 * CurrentPixelIndex] = BgColor.B; //b
				Pixels[4 * CurrentPixelIndex + 3] = 255; //set A channel always to maximum
			}
		}
		Mip.BulkData.Unlock();
		NewTexture->UpdateResource();
	}
}

void SLogViewerProWidgetMinimap::UpdateTexture(uint32 LogOffset)
{
	if (!NewTexture)
	{
		return;
	}

	if (!NewTexture->GetPlatformData())
	{
		return;
	}
	if (NewTexture->GetPlatformData()->Mips[0].BulkData.IsLocked())
	{
		return;
	}
	uint8* Pixels = static_cast<uint8*>(NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	if (!Pixels)
	{
		return;
	}

	BaseLogPos = LogOffset - TextureHeight / 2; //Stay in the middle

	if (IsStickToTop(LogOffset))
	{
		BaseLogPos = 0;
	}
	else if (IsStickToBottom(LogOffset))
	{
		BaseLogPos = MainWidget->LogMessages.Num() - TextureHeight;
	}

	const bool bColorfulMinimap = MainWidget->GetSettings()->IsShowColorfulMinimap();

	for (uint32 y = 0; y < TextureHeight; y++)
	{
		 
		const uint32 LogPos = y + BaseLogPos;
		for (uint32 x = 0; x < TextureWidth; x++)
		{
			int32 CurrentPixelIndex = ((y * TextureWidth) + x);
		
			FColor FinalColor = BgColor;
			if (MainWidget->LogMessages.IsValidIndex(LogPos))
			{
				if (MainWidget->LogListView && MainWidget->LogListView->IsItemSelected(MainWidget->LogMessages[LogPos]))
				{
					FinalColor = SelectedColor;

				}
				else if (MainWidget->LogMessages[LogPos]->bFound)
				{
					FinalColor = HighlightedColor;
				}
				else if (x < (uint32)MainWidget->LogMessages[LogPos]->Message->Len())
				{
					if (MainWidget->LogMessages[LogPos]->Verbosity == ELogVerbosity::Error)
					{
						FinalColor = ErrorColor;
					}
					else if (MainWidget->LogMessages[LogPos]->Verbosity == ELogVerbosity::Warning)
					{
						FinalColor = WarningColor;
					}
					else
					{
						if(bColorfulMinimap)
						{
							FinalColor = MainWidget->GetColorTable()->GetColorForCategory(MainWidget->LogMessages[LogPos]->Category).ToFColor(true);
						}
						else
						{
							FinalColor = FontColor;
						}
					}
				}
				else
				{
					if (MainWidget->LogListView && MainWidget->LogListView->IsItemVisible(MainWidget->LogMessages[LogPos]))
					{
						FinalColor = BgScreenColor;
					}
					else
					{
						FinalColor = BgColor;
					}
				}
			}

			Pixels[4 * CurrentPixelIndex + 2] = FinalColor.R; //r
			Pixels[4 * CurrentPixelIndex + 1] = FinalColor.G; //g
			Pixels[4 * CurrentPixelIndex] = FinalColor.B; //b
			Pixels[4 * CurrentPixelIndex + 3] = 255; //set A channel always to maximum
		}
	}
	NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	NewTexture->UpdateResource();

	bDirty = false; //why am I doing it here? because it's home project
}

bool SLogViewerProWidgetMinimap::IsStickToTop(uint32 LogOffset) const
{
	return (LogOffset < TextureHeight / 2u) || ((uint32)MainWidget->LogMessages.Num() < TextureHeight);
}

bool SLogViewerProWidgetMinimap::IsStickToBottom(uint32 LogOffset) const
{
	return (LogOffset > MainWidget->LogMessages.Num() - TextureHeight / 2u);
}

void SLogViewerProWidgetMinimap::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	Timer += InDeltaTime;
	if (!MainWidget->GetSettings()->IsShowMinimap())
	{
		return;
	}
	if (!bDirty)
	{
		return;
	}
	if (Timer > 0.2f)
	{
		Timer = 0.f;
		UpdateTexture((uint32)MainWidget->LogListView->GetScrollOffset());
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SLogViewerProWidgetMinimap::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector2D Position = MouseEvent.GetScreenSpacePosition() - MyGeometry.GetAbsolutePosition(); //@TODO support windows monitor scale
	float Scale = (float)TextureHeight / (float)MyGeometry.Size.Y;
	Position.Y *= Scale;
	bMouseDown = true;

	MainWidget->ScrollTo(BaseLogPos + FMath::RoundToInt(Position.Y));
	return FReply::Handled();
}

FReply SLogViewerProWidgetMinimap::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, TEXT("TEST: %s") + MyGeometry.Position.ToString(), true);
	bMouseDown = false;
	return FReply::Handled();
}

FReply SLogViewerProWidgetMinimap::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const int32 Scale = -10.f;
	MainWidget->ScrollRelative(MouseEvent.GetWheelDelta()*Scale);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE