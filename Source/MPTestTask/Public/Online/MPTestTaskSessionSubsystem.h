// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MPTestTaskSessionSubsystem.generated.h"

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
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostSession(int32 NumPublicConnections, const FString& ServerName);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions();
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSessionByIndex(int32 SearchIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

protected:
	IOnlineSessionPtr GetSessions() const;
	
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
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
};
