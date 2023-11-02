// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "LogViewerProOutputDevice.h"

class FToolBarBuilder;
class FMenuBuilder;
class SDockTab;

class FLogViewerProModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

//COMMANDS
	void PluginButtonClicked();

	//Cross tabs functionality - cosmetic only, main functionality is based on ini file state
	bool IsDefaultCategoriesEnabled() const { return bDefaultEnabled; }
	void DefaultCategoriesEnable() { bDefaultEnabled = true; }
	void DefaultCategoriesDisable() { bDefaultEnabled = false; }
	
private:

	void OnEditorClosed();

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> CreateLogViewerProPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	void RegisterDefaultLogViewerProTab(FTabManager& InTabManager);

	TSharedRef<SDockTab> SpawnEmptyLogViewerProTab();
	TSharedRef<SDockTab> SpawnDefaultLogViewerProTab(const FSpawnTabArgs& Args);


	TSharedRef<SDockTab> OnSpawnTabNewDocument(const FSpawnTabArgs& Args);

	void OnOpenNewTabClicked(TSharedRef<SDockTab> DockTab, ETabActivationCause InActivationCause);

	void OnSpecialTabClosed(TSharedRef<SDockTab> Tab);
	TSharedRef<SDockTab> OnSpawnTabNewDocument();

	TSharedPtr<class FUICommandList> GetPluginCommands() const { return PluginCommands; };



private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FLogViewerProOutputDevice> LogViewerProOutputDevice;

	TSharedPtr<class FTabManager> TabManager;
	TSharedPtr<FTabManager::FLayout> TabManagerLayout;

	static FName TabNameOpenNewTab;
	static FName TabNameDefaultEngineOutput;

	TWeakPtr<SDockTab> PluginTab;
	TWeakPtr<SDockTab> SpecialTab;

	bool bDefaultEnabled = false;
};
