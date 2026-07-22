// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Online/MPTestTaskSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Settings/MPTestTaskDevSettings.h"

DEFINE_LOG_CATEGORY(LogSession);

void UMPTestTaskSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMPTestTaskSessionSubsystem::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &UMPTestTaskSessionSubsystem::HandleTravelFailure);
	}
}

void UMPTestTaskSessionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
		GEngine->OnTravelFailure().RemoveAll(this);
	}
}

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
	
	const bool bEOS = IsUsingEOS();
	
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = !bEOS;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bUsesPresence = bEOS;
	LastSessionSettings->bAllowJoinViaPresence = bEOS;
	LastSessionSettings->bUseLobbiesIfAvailable = bEOS;
	
	LastSessionSettings->Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	if (bEOS)
	{
		LastSessionSettings->Set(FName("GAME_ID")
			, FString(TEXT("MPTestTask"))
			, EOnlineDataAdvertisementType::ViaOnlineService);
	}
	
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
	
	UE_LOG(LogSession, Display, TEXT("Searching for sessions..."));
	
	const bool bEOS = IsUsingEOS();
	
	LastSearch = MakeShareable(new FOnlineSessionSearch());
	LastSearch->bIsLanQuery = !bEOS;
	LastSearch->MaxSearchResults = 20;
	
	if (bEOS)
	{
		LastSearch->QuerySettings.Set(FName("GAME_ID")
			, FString(TEXT("MPTestTask"))
			, EOnlineComparisonOp::Equals);
	}
	
	OnFindSessionCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMPTestTaskSessionSubsystem::HandleFindSessionsComplete));
	
	Sessions->FindSessions(0, LastSearch.ToSharedRef());
}

void UMPTestTaskSessionSubsystem::JoinSessionByIndex(int32 SearchIndex)
{
	if (bIsJoining)
	{
		UE_LOG(LogSession, Warning, TEXT("Join already in progress"));
		return;
	}
	
	bIsJoining = true;
	
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
	
	const UMPTestTaskDevSettings* DevSettings = GetDefault<UMPTestTaskDevSettings>();
	if (!IsValid(DevSettings))
	{
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(JoinTimerHandle
			, this
			, &UMPTestTaskSessionSubsystem::HandleJoinTimeout
			, DevSettings->JoinTimeoutSeconds
			, false);
	}
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

void UMPTestTaskSessionSubsystem::LeaveSession()
{
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogSession, Display, TEXT("Leaving session"));
		Sessions->DestroySession(NAME_GameSession);
	}
	
	const UMPTestTaskDevSettings* DevSettings = GetDefault<UMPTestTaskDevSettings>();
	if (!IsValid(DevSettings))
	{
		return;
	}
	
	FString MenuPath = DevSettings->MainMenuMap.ToSoftObjectPath().GetLongPackageName();
	if (MenuPath.IsEmpty())
	{
		UE_LOG(LogSession, Error, TEXT("LeaveSession: MainMenuMap not set! Check project settings"));
		return;
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return;
	}
	
	if (APlayerController* PC = GameInstance->GetFirstLocalPlayerController())
	{
		PC->ClientTravel(MenuPath, TRAVEL_Absolute);
		
		UE_LOG(LogSession, Display, TEXT("%s left session")
			, PC->PlayerState ? *PC->PlayerState->GetPlayerName() : TEXT("null"));
	}
}

void UMPTestTaskSessionSubsystem::Login()
{
	if (!IsUsingEOS())
	{
		OnMPLoginComplete.Broadcast(true);
		return;
	}
	
	IOnlineSubsystem* OnlineSubsystem = GetActiveOSS();
	if (!OnlineSubsystem)
	{
		return;
	}
	
	IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		return;
	}
	
	if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		UE_LOG(LogSession, Display, TEXT("User already logged in"));
		OnMPLoginComplete.Broadcast(true);
		return;
	}
	
	OnLoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginCompleteDelegate::CreateUObject(this, &UMPTestTaskSessionSubsystem::HandleLoginComplete));
	
	const UMPTestTaskDevSettings* DevSettings = GetDefault<UMPTestTaskDevSettings>();
	if (!IsValid(DevSettings))
	{
		return;
	}
	
	// its for dev testing only - for ready game credentials should be changed properly
	FOnlineAccountCredentials Credentials;
	Credentials.Type = TEXT("Developer");
	Credentials.Id = TEXT("localhost:6300");
	Credentials.Token = DevSettings->DevAuthCredentialName;
	
	Identity->Login(0, Credentials);
}

bool UMPTestTaskSessionSubsystem::IsLoggedIn() const
{
	if (!IsUsingEOS())
	{
		return true;
	}
	
	IOnlineSubsystem* OnlineSubsystem = GetActiveOSS();
	if (!OnlineSubsystem)
	{
		return false;
	}
	
	IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
	
	return Identity.IsValid() && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

IOnlineSessionPtr UMPTestTaskSessionSubsystem::GetSessions() const
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	
	const IOnlineSubsystem* OnlineSubsystem = GetActiveOSS();
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

void UMPTestTaskSessionSubsystem::HandleNetworkFailure(UWorld* World
	, UNetDriver* NetDriver
	, ENetworkFailure::Type FailureType
	, const FString& ErrorString)
{
	UE_LOG(LogSession, Warning, TEXT("Network failure %s. Returning to main menu")
		, *ErrorString);	
	
	LeaveSession();
	
	if (World)
	{
		World->GetTimerManager().ClearTimer(JoinTimerHandle);
		JoinTimerHandle.Invalidate();
	}
}

void UMPTestTaskSessionSubsystem::HandleTravelFailure(UWorld* World
	, ETravelFailure::Type FailureType
	, const FString& ErrorString)
{
	UE_LOG(LogSession, Warning, TEXT("Travel failure %s"), *ErrorString);
	
	bIsJoining = false;
	
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}
	
	if (World)
	{
		World->GetTimerManager().ClearTimer(JoinTimerHandle);
		JoinTimerHandle.Invalidate();
	}
	
	OnMPJoinSessionComplete.Broadcast(false);
}

void UMPTestTaskSessionSubsystem::HandleJoinTimeout()
{
	UE_LOG(LogSession, Warning, TEXT("Join timed out. Refreshing server list"));
	
	bIsJoining = false;
	
	if (GEngine)
	{
		GEngine->CancelPending(GetWorld());
	}
	
	const IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}
	
	OnMPJoinSessionComplete.Broadcast(false);
	FindSessions();
}

void UMPTestTaskSessionSubsystem::HandleLoginComplete(int32 LocalUserNum
	, bool bWasSuccessful
	, const FUniqueNetId& UserId
	, const FString& Error)
{
	
	if (IOnlineSubsystem* OnlineSubsystem = GetActiveOSS())
	{
		IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			Identity->ClearOnLoginCompleteDelegate_Handle(0, OnLoginCompleteDelegateHandle);
		}
	}
	
	UE_CLOG(!bWasSuccessful, LogSession, Display, TEXT("EOS login failed %s"), *Error);
	UE_CLOG(bWasSuccessful, LogSession, Display, TEXT("EOS login successful %s"), *UserId.ToString());
	
	OnMPLoginComplete.Broadcast(bWasSuccessful);
}

IOnlineSubsystem* UMPTestTaskSessionSubsystem::GetActiveOSS() const
{
	return Online::GetSubsystem(GetWorld(), IsUsingEOS() ? EOS_SUBSYSTEM : NULL_SUBSYSTEM);
}

bool UMPTestTaskSessionSubsystem::IsUsingEOS() const
{
	const UMPTestTaskDevSettings* DevSettings = GetDefault<UMPTestTaskDevSettings>();
	if (!IsValid(DevSettings))
	{
		return false;
	}
	
	return DevSettings->bUseEOS;
}
