// Copyright 2019-Present tarnishablec. All Rights Reserved.


#include "Beeep/BeeepMessageSubsystem.h"

#include "GameplayTagContainer.h"
#include "Beeep/BeeepTypes.h"

DEFINE_LOG_CATEGORY(LogBeeepMessageSubsystem);

bool FBeeepMessageListenerHandle::IsValid() const
{
    return !Dead && Subsystem.IsValid() && ID != 0 && Channel.IsValid();
}

void FBeeepMessageListenerHandle::Unregister()
{
    if (!Subsystem.IsStale())
    {
        Subsystem.Get()->UnregisterListener(*this);
        Subsystem.Reset();
        Channel = FGameplayTag();
        ID = 0;
        MarkAsDead();
    }
}

void UBeeepMessageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UBeeepMessageSubsystem::Deinitialize()
{
    ListenerMap.Reset();
    Super::Deinitialize();
}

void UBeeepMessageSubsystem::BroadcastMessage(const FGameplayTag Channel, const FInstancedStruct& Payload)
{
    struct FInvalidLocation
    {
        FChannelListenerList* InvalidList;
        FGameplayTag InvalidChannel;
        int32 Index;
    };

    TArray<FInvalidLocation> InvalidLocations;

    bool bOnInitialTag = true;
    for (auto Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
    {
        if (auto* ListenerList = ListenerMap.Find(Tag))
        {
            for (int i = 0; i < ListenerList->Listeners.Num(); ++i)
            {
                const auto Listener = ListenerList->Listeners[i];
                if (Listener.IsValid())
                {
                    if (bOnInitialTag || Listener.MatchMode == EBeeepChannelMatchMode::PartialMatch)
                    {
                        Listener.MessageReceivedCallback(Channel, Payload);
                    }
                }
                else
                {
                    InvalidLocations.Add({ListenerList, Tag, i});
                }
            }
        }

        bOnInitialTag = false;
    }

    for (auto& [InvalidList,InvalidChannel, Index] : InvalidLocations)
    {
        if (InvalidList)
        {
            InvalidList->Listeners.RemoveAtSwap(Index);

            if (InvalidList->Listeners.Num() == 0)
            {
                ListenerMap.Remove(InvalidChannel);
            }
        }
    }
}

void UBeeepMessageSubsystem::RegisterListener(const FBeeepMessageListenerParams& Params,
                                              FBeeepMessageListenerHandle& Handle)
{
    auto& [Listeners, HandleID] = ListenerMap.FindOrAdd(Params.Channel);
    auto& Entry = Listeners.AddDefaulted_GetRef();

    Entry.Channel = Params.Channel;
    Entry.MatchMode = Params.MatchMode;
    Entry.MessageReceivedCallback = Params.MessageReceivedCallback;
    Entry.ID = ++HandleID;
    Entry.Subsystem = this;
    Entry.Dead = false;

    Handle = Entry;
}

void UBeeepMessageSubsystem::UnregisterListener(const FBeeepMessageListenerHandle& Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }


    if (auto* ListenerList = ListenerMap.Find(Handle.Channel))
    {
        const auto MatchedIndex = ListenerList->Listeners.IndexOfByPredicate(
            [ID = Handle.ID](const FBeeepMessageListenerHandle& InHandle)
            {
                return InHandle.ID == ID;
            });

        if (MatchedIndex != INDEX_NONE)
        {
            ListenerList->Listeners[MatchedIndex].MarkAsDead();
        }
    }
}
