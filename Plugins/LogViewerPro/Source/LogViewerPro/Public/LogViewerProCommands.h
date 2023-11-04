// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LogViewerProStyle.h"

class FLogViewerProCommands : public TCommands<FLogViewerProCommands>
{
public:

	FLogViewerProCommands()
		: TCommands<FLogViewerProCommands>(TEXT("LogViewerPro"), NSLOCTEXT("LogViewerPro", "LogViewerProPluginName", "LogViewerPro Plugin"), NAME_None, FLogViewerProStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};