// Copyright Dmitrii Labadin 2023

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "LogViewerProStructs.h"
#include "Serialization/JsonSerializerMacros.h"

#include "Widgets/Input/SComboButton.h"


class SLogViewerProWidgetMain;



class FLVSettingsData : public FJsonSerializable
{
public:
	FLVSettingsData() 
		: bShowTime(true)
		, bShowSeconds(true)
		, bShowFrame (true)
		, bCompactRow(false)
		, bShowMinimap(true)
		, bColorfulLogs(true)
		, bColorfulMinimap(true)
		, bCleanUpLogsOnPIE(false)
		{}

	virtual ~FLVSettingsData() {}

	bool bShowTime;
	bool bShowSeconds;
	bool bShowFrame;
	bool bCompactRow;
	bool bShowMinimap;
	bool bColorfulLogs;
	bool bColorfulMinimap;
	bool bCleanUpLogsOnPIE;

	// FJsonSerializable
	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE("ShowTimeColumn", bShowTime);
		JSON_SERIALIZE("ShowSecondsColumn", bShowSeconds);
		JSON_SERIALIZE("ShowFrameColumn", bShowFrame);
		JSON_SERIALIZE("CompactRow", bCompactRow);
		JSON_SERIALIZE("ShowMinimap", bShowMinimap);
		JSON_SERIALIZE("ColorfulLogs", bColorfulLogs);
		JSON_SERIALIZE("ColorfulMinimap", bColorfulMinimap);
		JSON_SERIALIZE("ColorfulMinimap", bCleanUpLogsOnPIE);
	END_JSON_SERIALIZER
};

class SLogViewerProSettingsButton : public SComboButton
{
public:

	SLATE_BEGIN_ARGS(SLogViewerProSettingsButton)
		: _MainWidget()
	{}
		SLATE_ARGUMENT(SLogViewerProWidgetMain*, MainWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	bool IsShowMinimap() const;
	bool IsShowTime() const;
	bool IsShowSeconds() const;
	bool IsShowFrame() const;
	bool IsShowFrameNum() const;
	bool IsCompactRow() const;
	bool IsShowLogColors() const;
	bool IsShowColorfulMinimap() const;
	bool IsCleanUpLogsOnPie() const;

	void TryLoadAndApplySettings();

private:

	SLogViewerProWidgetMain* MainWidget;

	TSharedRef<SWidget> GenerateSettingsMenu();
	void ToggleShowMinimap();
	void ToggleShowTime();
	void ToggleShowSeconds();
	void ToggleShowFrame();
	void ToggleCompactRow();
	void ToggleShowLogColors();
	void ToggleColorfulMinimap();
	void ToggleCleanUpLogsOnPie();

	void SaveSettings();
	void JoinDiscordServer();
	

	FString GetDefaultSettingsFolderPath() const;
	FString GetDefaultSettingsFilePath() const;

	FLVSettingsData SettingsData;
};
