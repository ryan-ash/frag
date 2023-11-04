// Copyright Dmitrii Labadin 2019

#include "LogViewerProCommands.h"

#define LOCTEXT_NAMESPACE "FLogViewerProModule"

void FLogViewerProCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "LogViewerPro", "Bring up LogViewerPro window", EUserInterfaceActionType::Button, FInputChord(EKeys::L, true, false, true, false));
}

#undef LOCTEXT_NAMESPACE
