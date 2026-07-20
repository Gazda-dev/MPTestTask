// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Online/MPTestTaskSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Settings/MPTestTaskDevSettings.h"

DEFINE_LOG_CATEGORY(LogSession);

void UMPTestTaskSessionSubsystem::HostSession(int32 NumPublicConnections
	, const FString& ServerName)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogSession, Error, TEXT("HostSession: Session interface is invalid!"));
		return;
	}
	
	const UMPTestTaskDevSettings* DevSettings = GetDefault<UMPTestTaskDevSettings>();
	if (!IsValid(DevSettings))
	{
		return;
	}
	
	FString MapPath = DevSettings->ArenaMap.ToSoftObjectPath().GetLongPackageName();
	if (MapPath.IsEmpty())
	{
		UE_LOG(LogSession, Error, TEXT("HostSession: MapPath is empty! Check project settings"));
		return;
	}
	
	UE_LOG(LogSession, Display, TEXT("Hosting session %s | slots: %d | map: %s")
		, *ServerName
		, NumPublicConnections
		, *MapPath);
	
	if (Sessions->GetNamedSession(NAME_GameSession) != nullptr)
	{
		UE_LOG(LogSession, Error, TEXT("Session already exists! Destroying it"));
		Sessions->DestroySession(NAME_GameSession);
	}
	
	PendingMapPath = MapPath;
	
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = true;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bUsesPresence = false;
	LastSessionSettings->bAllowJoinViaPresence = false;
	LastSessionSettings->bUseLobbiesIfAvailable = false;
	
	LastSessionSettings->Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMPTestTaskSessionSubsystem::HandleCreateSessionComplete));
	
	Sessions->CreateSession(0, NAME_GameSession, *LastSessionSettings);
}

void UMPTestTaskSessionSubsystem::FindSessions()
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogSession, Error, TEXT("FindSessions: Session interface is invalid!"));
		return;
	}
	
	UE_LOG(LogSession, Display, TEXT("Searching for LAN sessions..."));
	
	LastSearch = MakeShareable(new FOnlineSessionSearch());
	LastSearch->bIsLanQuery = true;
	LastSearch->MaxSearchResults = 20;
	
	OnFindSessionCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMPTestTaskSessionSubsystem::HandleFindSessionsComplete));
	
	Sessions->FindSessions(0, LastSearch.ToSharedRef());
}

void UMPTestTaskSessionSubsystem::JoinSessionByIndex(int32 SearchIndex)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions.IsValid() || !LastSearch.IsValid())
	{
		UE_LOG(LogSession, Error, TEXT("JoinSession aborted - session interface invalid or no search results"));
		return;
	}
	
	if (!LastSearch->SearchResults.IsValidIndex(SearchIndex))
	{
		UE_LOG(LogSession, Error, TEXT("JoinSession: invalid search index %d (results: %d")
			, SearchIndex
			, LastSearch->SearchResults.Num());
		return;
	}
	
	UE_LOG(LogSession, Display, TEXT("Joining session at index %d..."), SearchIndex);
	
	OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMPTestTaskSessionSubsystem::HandleJoinSessionComplete));
	
	Sessions->JoinSession(0, NAME_GameSession, LastSearch->SearchResults[SearchIndex]);
}

void UMPTestTaskSessionSubsystem::DestroySession()
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogSession, Display, TEXT("Destroying session"));
		Sessions->DestroySession(NAME_GameSession);
	}
}

IOnlineSessionPtr UMPTestTaskSessionSubsystem::GetSessions() const
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(World);
	if (!OnlineSubsystem)
	{
		UE_LOG(LogSession, Error, TEXT("OSS is not valid!"));
		return nullptr;
	}
	
	UE_LOG(LogSession, Display, TEXT("OnlineSubsystem is valid! Platform %s"), *OnlineSubsystem->GetSubsystemName().ToString());
	
	return OnlineSubsystem->GetSessionInterface();
}

void UMPTestTaskSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}
	
	OnMPCreateSessionComplete.Broadcast(bWasSuccessful);
	
	if (!bWasSuccessful)
	{
		UE_LOG(LogSession, Error, TEXT("Create session %s failed"), *SessionName.ToString());
		return;
	}
	
	const FString Travel = PendingMapPath + TEXT("?listen");
	
	UE_LOG(LogSession, Display, TEXT("Session %s created, Server travel -> %s")
		, *SessionName.ToString()
		, *Travel);
	
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(Travel);
	}
}

void UMPTestTaskSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionCompleteDelegateHandle);
	}
	
	TArray<FSessionInfo> Results;
	
	if (!bWasSuccessful)
	{
		UE_LOG(LogSession, Error, TEXT("FindSessions query failed!"));
	}
	else if (LastSearch.IsValid())
	{
		const int32 Found = LastSearch->SearchResults.Num();
		
		UE_CLOG(Found == 0, LogSession, Warning, TEXT("No sessions found"));
		UE_CLOG(Found > 0, LogSession, Display, TEXT("Found %d sessions"), Found);
		
		for (int32 It = 0; It < Found; ++It)
		{
			const FOnlineSessionSearchResult& Raw = LastSearch->SearchResults[It];
			
			FSessionInfo Info;
			Info.SearchIndex = It;
			Info.Ping = Raw.PingInMs;
			Info.MaxPlayers = Raw.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Raw.Session.NumOpenPublicConnections;
			
			FString ServerName;
			Raw.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);
			Info.ServerName = ServerName.IsEmpty() ? TEXT("unnamed") : ServerName;
			
			UE_LOG(LogSession, Display, TEXT("[%d] %s | %d/%d players | ping %d ms")
				, It
				, *Info.ServerName
				, Info.CurrentPlayers
				, Info.MaxPlayers
				, Info.Ping);
			
			Results.Add(Info);
		}
	}
	
	OnMPFindSessionComplete.Broadcast(bWasSuccessful, Results);
}

void UMPTestTaskSessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}
	
	const bool bSuccess = Result == EOnJoinSessionCompleteResult::Success;
	OnMPJoinSessionComplete.Broadcast(bSuccess);
	
	if (!bSuccess)
	{
		UE_LOG(LogSession, Error, TEXT("JoinSession %s failed"), *SessionName.ToString());
	}
	
	FString ConnectString;
	if (!Sessions->GetResolvedConnectString(NAME_GameSession, ConnectString))
	{
		UE_LOG(LogSession, Error, TEXT("JoinSession: could not resolve host adress"));
		return;
	}
	
	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}
	
	APlayerController* PC = GameInstance->GetFirstLocalPlayerController();
	if (!PC)
	{
		UE_LOG(LogSession, Error, TEXT("JoinSession: PlayerController not found!"))
		return;
	}
	
	UE_LOG(LogSession, Display, TEXT("Join successful. ClientTravel -> %s"), *ConnectString);
	PC->ClientTravel(ConnectString, TRAVEL_Absolute);
}
