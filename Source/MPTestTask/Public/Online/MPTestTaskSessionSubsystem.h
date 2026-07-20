// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MPTestTaskSessionSubsystem.generated.h"

namespace ETravelFailure
{
	enum Type : int;
}

namespace ENetworkFailure
{
	enum Type : int;
}

class UNetDriver;
DECLARE_LOG_CATEGORY_EXTERN(LogSession, Display, All);

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString ServerName;
	
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 SearchIndex = -1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMPOnFindSessionComplete, bool, bWasSuccessful, const TArray<FSessionInfo>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnJoinSessionComplete, bool, bWasSuccessful);

UCLASS()
class MPTESTTASK_API UMPTestTaskSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostSession(int32 NumPublicConnections, const FString& ServerName);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions();
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSessionByIndex(int32 SearchIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();
	
protected:
	IOnlineSessionPtr GetSessions() const;
	
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
	void HandleNetworkFailure(UWorld* World
		, UNetDriver* NetDriver
		, ENetworkFailure::Type FailureType
		, const FString& ErrorString);
	
	void HandleTravelFailure(UWorld* World
		, ETravelFailure::Type FailureType
		, const FString& ErrorString);
	
	void HandleJoinTimeout();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMPOnCreateSessionComplete OnMPCreateSessionComplete;
	
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMPOnFindSessionComplete OnMPFindSessionComplete;
	
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMPOnJoinSessionComplete OnMPJoinSessionComplete;
	
protected:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSearch;
	
	FString PendingMapPath;
	
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	
	bool bIsJoining = false;
	
	FTimerHandle JoinTimerHandle;
};
