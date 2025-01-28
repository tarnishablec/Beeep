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
    for (auto&& ListenerMapEntry : ListenerMap)
    {
        const auto Listeners = ListenerMapEntry.Value.Listeners;
        for (FBeeepMessageListenerHandle Listener : Listeners)
        {
            Listener.Unregister();
        }
    }
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

    check(Channel.IsValid());

    using FInvalidLocationNodeType = TSharedPtr<FInvalidLocation>;
    TLinkedList<FInvalidLocationNodeType>* InvalidLocationsStart = nullptr;

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
                        if (Listener.MessageReceived.IsBound())
                        {
                            Listener.MessageReceived.Execute(Channel, Payload);
                        }
                    }
                }
                else
                {
                    auto InvalidLocation = MakeShared<FInvalidLocation>(ListenerList, Tag, i);
                    auto* InvalidLocationNode = new TLinkedList<FInvalidLocationNodeType>(InvalidLocation);
                    InvalidLocationNode->LinkHead(InvalidLocationsStart);
                    InvalidLocationsStart = InvalidLocationNode;
                }
            }
        }

        bOnInitialTag = false;
    }

    if (auto* CurrentNode = InvalidLocationsStart)
    {
        while (CurrentNode)
        {
            auto* NextNode = CurrentNode->Next();
            auto& NodeData = **CurrentNode;

            if (NodeData->InvalidList)
            {
                NodeData->InvalidList->Listeners.RemoveAtSwap(NodeData->Index);
                if (NodeData->InvalidList->Listeners.Num() == 0)
                {
                    ListenerMap.Remove(NodeData->InvalidChannel);
                }
            }

            delete CurrentNode;
            NodeData.Reset();
            CurrentNode = NextNode;
        }
        InvalidLocationsStart = nullptr;
    }
}

void UBeeepMessageSubsystem::RegisterListener(const FBeeepMessageListenerParams& Params,
                                              FBeeepMessageListenerHandle& Handle)
{
    auto& [Listeners, HandleID] = ListenerMap.FindOrAdd(Params.Channel);
    auto& Entry = Listeners.AddDefaulted_GetRef();

    Entry.Channel = Params.Channel;
    Entry.MatchMode = Params.MatchMode;
    Entry.MessageReceived = Params.MessageReceived;
    Entry.ID = ++HandleID;
    Entry.Subsystem = this;
    Entry.Dead = false;

    Handle = Entry;
}

void UBeeepMessageSubsystem::UnregisterListener(const FBeeepMessageListenerHandle& Handle)
{
    if (!Handle.IsValid() || Handle.Subsystem != this)
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
