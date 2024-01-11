// Copyright Dmitrii Labadin 2019

#include "LogViewerPro.h"
#include "LogViewerProStyle.h"
#include "LogViewerProCommands.h"
#include "LogViewerProWidgetMain.h"
#include "Widgets/Docking/SDockTab.h"
#include "EditorStyleSet.h"
#include "LogViewerProOutputDevice.h"
#include "LevelEditor.h"
#include "LogViewerProStructs.h"


static const FName LogViewerProTabName("LogViewerPro");

#define LOCTEXT_NAMESPACE "FLogViewerProModule"

FName FLogViewerProModule::TabNameOpenNewTab(TEXT("TabNameOpenNewTab"));
FName FLogViewerProModule::TabNameDefaultEngineOutput(TEXT("TabNameDefaultEngineOutput"));

void FLogViewerProModule::StartupModule()
{
	LogViewerProOutputDevice = MakeShareable(new FLogViewerProOutputDevice);

	FLogViewerProStyle::Initialize();
	FLogViewerProStyle::ReloadTextures();

	FLogViewerProCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLogViewerProCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FLogViewerProModule::PluginButtonClicked),
		FCanExecuteAction());
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("Log", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FLogViewerProModule::AddMenuExtension));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Content", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLogViewerProModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LogViewerProTabName, FOnSpawnTab::CreateRaw(this, &FLogViewerProModule::CreateLogViewerProPluginTab))
		.SetDisplayName(LOCTEXT("FLogViewerProTabTitle", "LogViewerPro"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FLogViewerProModule::OnEditorClosed()
{
	//UE_LOG(LogTemp, Log, TEXT("Editor Closer"));
}


void FLogViewerProModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//GEditor->OnEditorClose().RemoveAll(this);

	FLogViewerProStyle::Shutdown();

	FLogViewerProCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LogViewerProTabName);
}

void FLogViewerProModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LogViewerProTabName);
}

void FLogViewerProModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FLogViewerProCommands::Get().OpenPluginWindow);
}

void FLogViewerProModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FLogViewerProCommands::Get().OpenPluginWindow);
}

TSharedRef<SDockTab> FLogViewerProModule::CreateLogViewerProPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTab> NomadTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(NSLOCTEXT("LogViewerPro", "MainTabTitle", "Log Viewer Pro"));
	PluginTab = NomadTab;
	if (!TabManager.IsValid())
	{
		TabManager = FGlobalTabmanager::Get()->NewTabManager(NomadTab);
	}

	TWeakPtr<FTabManager> TabManagerWeak = TabManager;
	// On tab close will save the layout if the debugging window itself is closed,
	// this handler also cleans up any floating debugging controls. If we don't close
	// all areas we need to add some logic to the tab manager to reuse existing tabs:
	NomadTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateStatic(
		[](TSharedRef<SDockTab> Self, TWeakPtr<FTabManager> TabManager)
		{
			TSharedPtr<FTabManager> OwningTabManager = TabManager.Pin();
			if (OwningTabManager.IsValid())
			{
				FLayoutSaveRestore::SaveToConfig(GEditorLayoutIni, OwningTabManager->PersistLayout());
				OwningTabManager->CloseAllAreas();
			}
		}
		, TabManagerWeak
		));

	if (!TabManagerLayout.IsValid())
	{
		TabManager->RegisterTabSpawner(TabNameOpenNewTab, FOnSpawnTab::CreateRaw(this, &FLogViewerProModule::OnSpawnTabNewDocument));
		RegisterDefaultLogViewerProTab(*TabManager);

		TabManagerLayout = FTabManager::NewLayout("LogViewerProLayout")
			->AddArea
			(
				FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Vertical)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(.4f)
					->SetHideTabWell(false)
					->AddTab(TabNameOpenNewTab, ETabState::OpenedTab)
					->AddTab(TabNameDefaultEngineOutput, ETabState::OpenedTab)
					->SetForegroundTab(TabNameDefaultEngineOutput)
				)
			);

		GEditor->OnEditorClose().AddRaw(this, &FLogViewerProModule::OnEditorClosed);
	}

	TSharedRef<SWidget> TabContents = TabManager->RestoreFrom(TabManagerLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();

	NomadTab->SetContent(
		TabContents
	);

	return NomadTab;
}

void FLogViewerProModule::RegisterDefaultLogViewerProTab(FTabManager& InTabManager)
{
#if 0
	const auto SpawnDefaultLogViewerProTab = [](const FSpawnTabArgs& Args)
	{
		TSharedRef<SLogViewerProWidgetMain> Widget = SNew(SLogViewerProWidgetMain).Messages(OutputLogHistory->GetMessages());
		TSharedRef<SDockTab> Tab = SNew(SDockTab)
			.TabRole(ETabRole::DocumentTab)
			.Label(NSLOCTEXT("LogViewerPro", "EngineOutputTabTitle", "Engine Output"))
			[
				Widget
			];
		Widget->SetParentTab(Tab);
		return Tab;
	};

#endif
	//InTabManager.RegisterTabSpawner(TabNameDefaultEngineOutput, FOnSpawnTab::CreateStatic(SpawnDefaultLogViewerProTab))
	InTabManager.RegisterTabSpawner(TabNameDefaultEngineOutput, FOnSpawnTab::CreateRaw(this, &FLogViewerProModule::SpawnDefaultLogViewerProTab))
		.SetDisplayName(NSLOCTEXT("LogViewerPro", "EngineOutputTabTitle", "Engine Output"))
		.SetTooltipText(NSLOCTEXT("LogViewerPro", "EngineOutputTabTitle", "Engine Output"));
}

TSharedRef<SDockTab> FLogViewerProModule::SpawnDefaultLogViewerProTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SLogViewerProWidgetMain> Widget = SNew(SLogViewerProWidgetMain).LogOutputDevice(LogViewerProOutputDevice).Module(this);
	TSharedRef<SDockTab> Tab = SNew(SDockTab)
		.TabRole(ETabRole::DocumentTab)
		.Label(NSLOCTEXT("LogViewerPro", "EngineOutputTabTitle", "Engine Output"))
		[
			Widget
		];
	Widget->SetParentTab(Tab);
	return Tab;
}

TSharedRef<SDockTab> FLogViewerProModule::OnSpawnTabNewDocument(const FSpawnTabArgs& Args)
{
	return OnSpawnTabNewDocument();
};

void FLogViewerProModule::OnSpecialTabClosed(TSharedRef<SDockTab> Tab)
{
	if (SpecialTab.IsValid())
	{
		//Clear callback so we are not in the loop
		SpecialTab.Pin()->SetOnTabClosed(SDockTab::FOnTabClosedCallback());
	}
	if (PluginTab.IsValid())
	{
		TSharedPtr<SWindow> ParentWindowPtr = PluginTab.Pin()->GetParentWindow();
		ParentWindowPtr->RequestDestroyWindow();
	}
}

TSharedRef<SDockTab> FLogViewerProModule::OnSpawnTabNewDocument()
{
	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::DocumentTab)
		.OnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FLogViewerProModule::OnSpecialTabClosed))
		.Label(NSLOCTEXT("LogVeiwerPro", "NewTabTabTitle", "Create New Tab"))
		.ToolTipText(NSLOCTEXT("LogVeiwerPro", "NewLogTabTitleToolTip", "Creates new empty log viewer tab"))
		[
				SNew(SBox)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Type::Right)
					.Text(FText::FromString(TEXT("Log Viewer Pro v1.239\r\nby Dmitrii Labadin\r\nYour support matters!")))
				]
		];
	NewTab->SetOnTabActivated(SDockTab::FOnTabActivatedCallback::CreateRaw(this, &FLogViewerProModule::OnOpenNewTabClicked));
	SpecialTab = NewTab;
	return NewTab;
}

TSharedRef<SDockTab> FLogViewerProModule::SpawnEmptyLogViewerProTab()
{
	TSharedRef<SLogViewerProWidgetMain> Widget = SNew(SLogViewerProWidgetMain).LogOutputDevice(LogViewerProOutputDevice).Module(this);
	TSharedRef<SDockTab> Tab = SNew(SDockTab)
		.TabRole(ETabRole::DocumentTab)
		.Label(NSLOCTEXT("LogViewerPro", "EngineOutputTabTitle", "Engine Output"))
		[
			Widget
		];
	Widget->SetParentTab(Tab);
	return Tab;
}

void FLogViewerProModule::OnOpenNewTabClicked(TSharedRef<SDockTab> DockTab, ETabActivationCause InActivationCause)
{
	TabManager->InsertNewDocumentTab(TabNameOpenNewTab, FTabManager::ESearchPreference::PreferLiveTab, SpawnEmptyLogViewerProTab());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogViewerProModule, LogViewerPro)