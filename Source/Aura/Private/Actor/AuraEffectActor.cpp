// Copyright Phoenix Head Game Studios


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* Actor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (ActorASC)
	{
		check(GameplayEffectClass);
		FGameplayEffectContextHandle EffectContextHandle = ActorASC->MakeEffectContext();
		EffectContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle EffectSpecHandle = ActorASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
		const FActiveGameplayEffectHandle ActiveGameplayEffectHandle = ActorASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

		// Checks if the effect is infinite and adds it to the ActiveEffectsHandles array
		if(EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite &&
			InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnOverlap || InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
			 ActiveEffectsHandles.Add( ActiveGameplayEffectHandle, ActorASC );
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	// Instant Effect
	if(InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
    {
        ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
    }

	// Duration Effect
	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
    {
        ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
    }
	/*if(DurationEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnOverlap)
    {
    // TODO: Implement the removal of the effect for the DurationEffectRemovalPolicy
		// Removes the effect
		UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if(!IsValid(ActorASC)) return;
    }*/

	// Infinite Effect
	if(InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
    {
        ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
    }
	if(InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnOverlap)
    {
        // Removes the effect
        RemoveEffectFromTarget(TargetActor);
    }
	
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	// Instant Effect
	if(InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
    {
        ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
    }

	// Duration Effect
	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
    {
        ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
    }

	// Infinite Effect
	if(InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
    {
        ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
    }
	if(InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
    {
        RemoveEffectFromTarget(TargetActor);
    }
}

#pragma region RemoveEffectFromTarget

void AAuraEffectActor::RemoveEffectFromTarget(AActor* TargetActor)
{
	// Removes the effect
	UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if(!IsValid(ActorASC)) return;
		
	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	for (auto HandlePair : ActiveEffectsHandles)
	{
		if(ActorASC == HandlePair.Value)
		{
			ActorASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
			HandlesToRemove.Add(HandlePair.Key);
		}
	}
	for (auto& Handle : HandlesToRemove)
	{
		ActiveEffectsHandles.FindAndRemoveChecked(Handle);
	}
}

#pragma endregion

