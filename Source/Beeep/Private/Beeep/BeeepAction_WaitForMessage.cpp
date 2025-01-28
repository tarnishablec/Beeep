// Copyright 2019-Present tarnishablec. All Rights Reserved.


#include "Beeep/BeeepAction_WaitForMessage.h"

#include "Beeep/BeeepMessageSubsystem.h"

UBeeepAction_WaitForMessage* UBeeepAction_WaitForMessage::ListenForMessage(
    UObject* WorldContextObject,
    const FGameplayTag Channel,
    const EBeeepChannelMatchMode MatchMode,
    FBeeepMessageListenerHandle& Handle
)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    const auto Action = NewObject<UBeeepAction_WaitForMessage>();

    Action->ListenerHandle = &Handle;
    Action->WorldPtr = World;
    Action->ActionMatchMode = MatchMode;
    Action->ChannelToRegister = Channel;
    Action->RegisterWithGameInstance(World);

    return Action;
}

void UBeeepAction_WaitForMessage::Activate()
{
    if (auto* Beeep = UBeeepMessageSubsystem::Get(WorldPtr.Get()))
    {
        TWeakObjectPtr WeakThis(this);
        FBeeepMessageListenerParams ListenerParams;
        ListenerParams.MatchMode = ActionMatchMode;
        ListenerParams.Channel = ChannelToRegister;
        ListenerParams.MessageReceived.BindDynamic(this, &UBeeepAction_WaitForMessage::HandleMessageReceived);

        Beeep->RegisterListener(ListenerParams, *ListenerHandle);
        return;
    }
    SetReadyToDestroy();
}

void UBeeepAction_WaitForMessage::SetReadyToDestroy()
{
    if (ListenerHandle)
    {
        ListenerHandle->Unregister();
        ListenerHandle = nullptr;
    }
    Super::SetReadyToDestroy();
}

void UBeeepAction_WaitForMessage::Cancel()
{
    Super::Cancel();
}

bool UBeeepAction_WaitForMessage::IsActive() const
{
    return Super::IsActive();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UBeeepAction_WaitForMessage::HandleMessageReceived(const FGameplayTag Channel, const FInstancedStruct& Payload)
{
    OnMessageReceived.Broadcast(Channel, Payload);
}
