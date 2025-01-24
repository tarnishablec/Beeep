// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BeeepTypes.generated.h"


struct FInstancedStruct;

// Match rule for message listeners
UENUM(BlueprintType)
enum class EBeeepChannelMatchMode : uint8
{
    // An exact match will only receive messages with exactly the same channel
    // (e.g., registering for "A.B" will match a broadcast of A.B but not A.B.C)
    ExactMatch,

    // A partial match will receive any messages rooted in the same channel
    // (e.g., registering for "A.B" will match a broadcast of A.B as well as A.B.C)
    PartialMatch
};

using FBeeepMessageCallback = TFunction<void(FGameplayTag, const FInstancedStruct&)>;

USTRUCT()
struct FBeeepMessageListenerParams
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    FGameplayTag Channel;

    UPROPERTY(Transient)
    EBeeepChannelMatchMode MatchMode = EBeeepChannelMatchMode::ExactMatch;

    FBeeepMessageCallback MessageReceivedCallback;
};
