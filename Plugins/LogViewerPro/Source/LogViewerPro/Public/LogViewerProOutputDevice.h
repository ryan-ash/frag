// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "LogViewerProStructs.h"
#include "Misc/OutputDevice.h"

class SLogViewerProWidgetMain;

class FLogViewerProOutputDevice 
	: public FOutputDevice
{

public:
	FLogViewerProOutputDevice();

	FDelegateHandle SubscribeOnMessageReceived(SLogViewerProWidgetMain* MainWidget);
	void UnsibscribeOnMessageReceived(FDelegateHandle& Handle);
	bool IsListening(const SLogViewerProWidgetMain* MainWidget);

	~FLogViewerProOutputDevice();

	TArray<TSharedPtr<FLogMessage>> Messages;

	//static TSharedPtr<FLogMessage> CreateLogFromString(const FString& LogMessage);
	static bool CreateLogMessages(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, TArray< TSharedPtr<FLogMessage> >& OutMessages);

	void ProcessBufferedLines();

protected:
	FOnLogMessageReceivedDelegate OnLogMessageReceivedDelegate;

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category ) override;
	virtual bool CanBeUsedOnAnyThread() const;

	/* Remove itself on crash to prevent adding log lines here */
	void OnCrash();

	/** Synchronization object for access to buffered lines */
	FCriticalSection		BufferedLinesSynch;
	TArray<FBufferedLine>	BufferedLines;

};
