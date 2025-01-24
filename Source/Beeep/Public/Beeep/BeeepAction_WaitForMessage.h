// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BeeepMessageSubsystem.h"
#include "Beeep/BeeepTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "BeeepAction_WaitForMessage.generated.h"

struct FInstancedStruct;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBeeepWaitForMessageDelegate, FGameplayTag, Channel,
                                             const FInstancedStruct&, Payload);

/**
 * 
 */
UCLASS()
class BEEEP_API UBeeepAction_WaitForMessage : public UCancellableAsyncAction
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintInternalUseOnly, BlueprintCallable, Category="Beeep", meta=(WorldContext="WorldContextObject"),
        DisplayName="Beeep Listen For Message")
    static UBeeepAction_WaitForMessage* ListenForMessage(
        UObject* WorldContextObject, FGameplayTag Channel,
        EBeeepChannelMatchMode MatchMode,
        FBeeepMessageListenerHandle& Handle
    );

    //
    virtual void Activate() override;
    virtual void SetReadyToDestroy() override;
    virtual void Cancel() override;
    virtual bool IsActive() const override;
    //

    FBeeepMessageListenerHandle* ListenerHandle;

    UPROPERTY(BlueprintAssignable)
    FBeeepWaitForMessageDelegate OnMessageReceived;

protected:
    TWeakObjectPtr<UWorld> WorldPtr;
    FGameplayTag ChannelToRegister;
    EBeeepChannelMatchMode ActionMatchMode;
};
