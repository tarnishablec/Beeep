// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Beeep/BeeepTypes.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BeeepMessageSubsystem.generated.h"

enum class EBeeepChannelMatchMode : uint8;
struct FBeeepMessageListenerParams;
struct FBeeepMessageListenerHandle;
struct FBeeepMessageListenerData;
struct FInstancedStruct;
struct FGameplayTag;
class UBeeepMessageSubsystem;

BEEEP_API DECLARE_LOG_CATEGORY_EXTERN(LogBeeepMessageSubsystem, Log, All);

/**
 * An opaque handle that can be used to remove a previously registered message listener
 * @see UBeeepMessageSubsystem::RegisterListener and UBeeepMessageSubsystem::UnregisterListener
 */
USTRUCT(BlueprintType)
struct BEEEP_API FBeeepMessageListenerHandle : public FBeeepMessageListenerParams
{
public:
    GENERATED_BODY()

    FBeeepMessageListenerHandle()
    {
    }

    bool IsValid() const;

    void Unregister();

    void MarkAsDead()
    {
        Dead = true;
    };

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<UBeeepMessageSubsystem> Subsystem;

    UPROPERTY(Transient)
    int32 ID = 0;

    UPROPERTY(Transient)
    bool Dead = false;

    friend UBeeepMessageSubsystem;
};


/**
 * 
 */
UCLASS()
class BEEEP_API UBeeepMessageSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Beeep", meta=(AutoCreateRefTerm = "Payload"))
    void BroadcastMessage(
        FGameplayTag Channel,
        const FInstancedStruct& Payload = FInstancedStruct()
    );

    UFUNCTION(BlueprintCallable, Category = "Beeep")
    void RegisterListener(const FBeeepMessageListenerParams& Params,
                          FBeeepMessageListenerHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Beeep")
    void UnregisterListener(const FBeeepMessageListenerHandle& Handle);

    static UBeeepMessageSubsystem* Get(const UObject* WorldContextObject)
    {
        const auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
        return World->GetGameInstance()->GetSubsystem<UBeeepMessageSubsystem>();
    };

protected:
    struct FChannelListenerList
    {
        TArray<FBeeepMessageListenerHandle> Listeners;
        int32 HandleID = 0;
    };

    TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};
