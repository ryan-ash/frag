// Copyright Dmitrii Labadin 2019

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "Misc/OutputDeviceHelper.h"
#include "Serialization/JsonSerializerMacros.h"

struct FLogMessage
{
	FDateTime Time = FDateTime();
	double TimeSeconds = 0.;
	int32 FrameNumber = 0;
	FName Category = TEXT("None");
	ELogVerbosity::Type Verbosity = ELogVerbosity::Log;
	TSharedRef<FString> Message;

	bool bNoTime = false;
	bool bFound = false;

	FLogMessage() {};

	FLogMessage(const FString& NewMessage)
		: Message(MakeShared<FString>(NewMessage))
		, bNoTime(true)
	{

	}
	
	FLogMessage(const FString& NewMessage, FName NewCategory, FName NewStyle = NAME_None)
		: Category(NewCategory)
		, Verbosity(ELogVerbosity::Log)
		, Message( MakeShared<FString>( NewMessage) )
	{
		Time = FDateTime::Now();
		FrameNumber = GFrameNumber;
		TimeSeconds = FPlatformTime::Seconds() - GStartTime;
		
	}

	FLogMessage(const FString& NewMessage, ELogVerbosity::Type NewVerbosity, FName NewCategory, FName NewStyle = NAME_None)
		: Category(NewCategory)
		, Verbosity(NewVerbosity)
		, Message(MakeShared<FString>(NewMessage))
	{
		Time = FDateTime::Now();
		FrameNumber = GFrameNumber;
		TimeSeconds = FPlatformTime::Seconds() - GStartTime;
	}

	FLogMessage(FDateTime InTime, double InTimeSeconds, int32 InFrameNumber, FName NewCategory, const FString& NewMessage, FName NewStyle = NAME_None)
		: Time(InTime)
		, TimeSeconds(InTimeSeconds)
		, FrameNumber(InFrameNumber)
		, Category(NewCategory)
		, Verbosity(ELogVerbosity::Log)
		, Message(MakeShared<FString>(NewMessage))
	{
	}

	FString AsString(bool bWithTime) const
	{
		const FString VerbosityString = ((Verbosity == ELogVerbosity::Log) ? *FString() : *(ToString(Verbosity) + FString(TEXT(": "))));
		if (bNoTime || !bWithTime )
		{
			if (Category.IsNone())
			{
				return FString::Printf(TEXT("%s%s"),
					*VerbosityString,
					//LogMessage->TimeSeconds,
					*Message.Get());
			}
			else
			{
				return FString::Printf(TEXT("%s: %s%s"),
					*Category.ToString(),
					*VerbosityString,
					//LogMessage->TimeSeconds,
					*Message.Get());
			}
		}
		else
		{
			return FString::Printf(TEXT("[%s][%3i]%s: %s%s"),
				*GetTimeFullString(),
				FrameNumber % 1000,
				*Category.ToString(),
				*VerbosityString,
				//LogMessage->TimeSeconds,
				*Message.Get());
		}
	}

	FString GetTimeHoursString() const
	{
		return Time.ToString(TEXT("%H:%M:%S.%s"));;
	}


	FString GetTimeFullString() const
	{
		return Time.ToString(TEXT("%Y.%m.%d-%H.%M.%S:%s"));
	}


};

class FLogColorTable
{
public:

	static FLinearColor GenerateRandomColor()
	{
		return FLinearColor(FMath::FRandRange(0.15f, 0.7f), FMath::FRandRange(0.15f, 1.f), FMath::FRandRange(0.15f, 1.f));
	}
	void RegisterCategory(FName Category)
	{
		ColorMap.Add( {Category, GenerateRandomColor()});
	}

	FLinearColor GetColorForCategory(FName Category) const
	{
		auto* Color = ColorMap.Find(Category);
		if (!Color)
		{
			FLinearColor(0.8f, 0.8f, 0.8f); //In case can't find color for category, use some greyish
		}
		return *Color;
		//return FLinearColor(FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f), FMath::FRandRange(0.f, 1.f));
	}

	void SetColorForCategory(FName Category, const FLinearColor& InColor)
	{
		if (!ColorMap.Contains(Category))
		{
			return;
		}
		ColorMap[Category] = FLinearColor(InColor.R, InColor.G, InColor.B, 1.f); //Override whatever alpha we receive, because color picker returns 0.f for example
	}

private:
	TMap<FName, FLinearColor> ColorMap;

};


class FLVCategoriesSaveItem : public FJsonSerializable
{
public:
	FLVCategoriesSaveItem()
		: CategoryName()
		, bIsActive(true)
		, CategoryHexColor()
	{}

	FLVCategoriesSaveItem(const FString& InCategoryName, bool bInIsActive, const FString& InCategoryHexColor)
		: CategoryName(InCategoryName)
		, bIsActive(bInIsActive)
		, CategoryHexColor(InCategoryHexColor)
	{}

	virtual ~FLVCategoriesSaveItem() {}

	FString CategoryName;
	bool bIsActive;
	FString CategoryHexColor;

	// FJsonSerializable
	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE("CategoryName", CategoryName);
		JSON_SERIALIZE("IsActive", bIsActive);
		JSON_SERIALIZE("ColorHex", CategoryHexColor);
	END_JSON_SERIALIZER
};

class FLVCategoriesSave : public FJsonSerializable
{
public:
	FLVCategoriesSave()
	{}
	virtual ~FLVCategoriesSave() {}

	TArray< FLVCategoriesSaveItem > Categories;

	// FJsonSerializable
	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE_ARRAY_SERIALIZABLE("Categories", Categories, FLVCategoriesSaveItem);
	END_JSON_SERIALIZER
};


DECLARE_MULTICAST_DELEGATE_OneParam(FOnLogMessageReceivedDelegate, TSharedPtr<FLogMessage>);