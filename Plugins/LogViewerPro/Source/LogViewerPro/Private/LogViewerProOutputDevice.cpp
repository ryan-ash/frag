// Copyright Dmitrii Labadin 2019

#include "LogViewerProOutputDevice.h"
#include "Misc/OutputDeviceHelper.h"
#include "Misc/CoreDelegates.h"
#include "CoreGlobals.h"
#include "LogViewerProWidgetMain.h"

#define LOCTEXT_NAMESPACE "SOutputLog"

FLogViewerProOutputDevice::FLogViewerProOutputDevice()
	: FOutputDevice()
{
	FCoreDelegates::OnHandleSystemError.AddRaw(this, &FLogViewerProOutputDevice::OnCrash);
	GLog->AddOutputDevice(this);
}

FDelegateHandle FLogViewerProOutputDevice::SubscribeOnMessageReceived(SLogViewerProWidgetMain* MainWidget)
{
	return OnLogMessageReceivedDelegate.AddSP(MainWidget, &SLogViewerProWidgetMain::HandleLogReceived);
}

void FLogViewerProOutputDevice::UnsibscribeOnMessageReceived(FDelegateHandle& Handle)
{
	//OnLogMessageReceivedDelegate.Unbind();
	OnLogMessageReceivedDelegate.Remove(Handle);
}

bool FLogViewerProOutputDevice::IsListening(const SLogViewerProWidgetMain* MainWidget)
{
	return OnLogMessageReceivedDelegate.IsBoundToObject(MainWidget);
}

FLogViewerProOutputDevice::~FLogViewerProOutputDevice()
{
	if (GLog != nullptr)
	{
		GLog->RemoveOutputDevice(this);
	}
	FCoreDelegates::OnHandleSystemError.RemoveAll(this);
}

void FLogViewerProOutputDevice::OnCrash()
{
	if (GLog != nullptr)
	{
		GLog->RemoveOutputDevice(this);
	}
}

//TSharedPtr<FLogMessage> FLogViewerProOutputDevice::CreateLogFromString(const FString& LogMessage)
//{
//	return MakeShared(FLogMessage());
//}

bool FLogViewerProOutputDevice::CreateLogMessages( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, TArray< TSharedPtr<FLogMessage> >& OutMessages )
{
	if (Verbosity == ELogVerbosity::SetColor)
	{
		// Skip Color Events
		return false;
	}

		// Determine how to format timestamps
	static ELogTimes::Type LogTimestampMode = ELogTimes::None;
	if (UObjectInitialized() && !GExitPurge)
	{
		// Logging can happen very late during shutdown, even after the UObject system has been torn down, hence the init check above
		//LogTimestampMode = GetDefault<UEditorStyleSettings>()->LogTimestampMode;
	}

	const int32 OldNumMessages = OutMessages.Num();

	// handle multiline strings by breaking them apart by line
	TArray<FTextRange> LineRanges;
	FString CurrentLogDump = V;
	FTextRange::CalculateLineRangesFromString(CurrentLogDump, LineRanges);

	bool bIsFirstLineInMessage = true;
	for (const FTextRange& LineRange : LineRanges)
	{
		if (!LineRange.IsEmpty())
		{
			FString Line = CurrentLogDump.Mid(LineRange.BeginIndex, LineRange.Len());
			Line = Line.ConvertTabsToSpaces(4);

			// Hard-wrap lines to avoid them being too long
			static const int32 HardWrapLen = 360;
			for (int32 CurrentStartIndex = 0; CurrentStartIndex < Line.Len();)
			{
				int32 HardWrapLineLen = 0;
				if (bIsFirstLineInMessage)
				{
					FString MessagePrefix = FOutputDeviceHelper::FormatLogLine(Verbosity, Category, nullptr, LogTimestampMode);

					HardWrapLineLen = FMath::Min(HardWrapLen - MessagePrefix.Len(), Line.Len() - CurrentStartIndex);
					FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);


					//OutMessages.Add(MakeShared<FLogMessage>(MakeShared<FString>(MessagePrefix + HardWrapLine), Verbosity, Category, Style));
					OutMessages.Add(MakeShared<FLogMessage>(MessagePrefix + HardWrapLine, Verbosity, Category));
				}
				else
				{
					HardWrapLineLen = FMath::Min(HardWrapLen, Line.Len() - CurrentStartIndex);
					FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

					//OutMessages.Add(MakeShared<FLogMessage>(MakeShared<FString>(MoveTemp(HardWrapLine)), Verbosity, Category, Style));
					OutMessages.Add(MakeShared<FLogMessage>(HardWrapLine, Verbosity, Category));
				}

				bIsFirstLineInMessage = false;
				CurrentStartIndex += HardWrapLineLen;
			}
		}
	}

	//HandleNewMessageCreated();

	return OldNumMessages != OutMessages.Num();
}

void FLogViewerProOutputDevice::ProcessBufferedLines()
{
	FScopeLock ScopeLock(&BufferedLinesSynch);
	if (BufferedLines.Num() > 0)
	{
		for (const FBufferedLine& Line : BufferedLines)
		{
			//OnLogMessageReceivedDelegate.ExecuteIfBound(MakeShared<FLogMessage>(FString(Line.Data), Line.Verbosity, Line.Category, FName(TEXT("Log.Command"))));
			//OnLogMessageReceivedDelegate.Broadcast(MakeShared<FLogMessage>(FString(Line.Data), Line.Verbosity, Line.Category, FName(TEXT("Log.Command"))));
			OnLogMessageReceivedDelegate.Broadcast(MakeShared<FLogMessage>(FString(Line.Data), Line.Verbosity, Line.Category, FName(TEXT("Log.Command"))));
			//MessagesTextMarshaller->AppendPendingMessage(Line.Data, Line.Verbosity, Line.Category);
		}
		BufferedLines.Empty(32);
	}
}

void FLogViewerProOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	FScopeLock ScopeLock(&BufferedLinesSynch);
	BufferedLines.Emplace(V, Category, Verbosity);
}


bool FLogViewerProOutputDevice::CanBeUsedOnAnyThread() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE

