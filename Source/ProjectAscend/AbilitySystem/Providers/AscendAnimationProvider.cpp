#include "AscendAnimationProvider.h"
#include "AbilitySystem/Abilities/AscendGameplayAbility.h"
#include "Interfaces/Animation/AnimationContextProviderInterface.h"
#include "Interfaces/Animation/AnimationMontageProviderInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAnimationProvider)

UAscendAnimationProvider::UAscendAnimationProvider()
{
    DefaultPlayRate = 1.0f;
    RootMotionScale = 1.0f;
    DefaultSectionName = NAME_None;
    DefaultAnimationMontage = nullptr;
}

UAnimMontage* UAscendAnimationProvider::GetMontageToPlay_Implementation(UAscendGameplayAbility* AscendGameplayAbility) const
{
    // Prefer source object (e.g. weapon) montage, then ability-level montage, then provider default.
    UAnimMontage* MontageFromAbility = GetAbilityAnimationMontage(AscendGameplayAbility);
    if (MontageFromAbility)
    {
        return MontageFromAbility;
    }
    return DefaultAnimationMontage;
}

FName UAscendAnimationProvider::GetSectionName_Implementation(UAscendGameplayAbility* AscendGameplayAbility) const
{
    FName SectionFromAbility = GetAbilitySectionName(AscendGameplayAbility);
    if (SectionFromAbility != NAME_None)
        return SectionFromAbility;

    return DefaultSectionName;
}

float UAscendAnimationProvider::GetPlayRate_Implementation(UAscendGameplayAbility* AscendGameplayAbility) const
{
    float RateFromAbility = GetAbilityPlayRate(AscendGameplayAbility);
    if (RateFromAbility > 0.0f)
        return RateFromAbility;

    return DefaultPlayRate;
}

float UAscendAnimationProvider::GetRootMotionScale_Implementation(UAscendGameplayAbility* AscendGameplayAbility) const
{
    return RootMotionScale;
}

UAnimMontage* UAscendAnimationProvider::GetAbilityAnimationMontage(const UAscendGameplayAbility* AscendGameplayAbility)
{
    if (!IsValid(AscendGameplayAbility))
    {
        return nullptr;
    }

    const FGameplayTagContainer AbilityTags = GetAbilityTags(AscendGameplayAbility);

    // Check source object first (weapons/items may override animation).
    if (UObject* SourceObject = AscendGameplayAbility->GetAbilitySourceObject())
    {
        if (UAnimMontage* SourceMontage = GetAnimationMontageFromObject(SourceObject, AbilityTags))
        {
            return SourceMontage;
        }
    }
    return GetAnimationMontageFromObject(AscendGameplayAbility, AbilityTags);
}

FName UAscendAnimationProvider::GetAbilitySectionName(const UAscendGameplayAbility* AscendGameplayAbility)
{
    if (!IsValid(AscendGameplayAbility))
    {
        return NAME_None;
    }

    const FGameplayTagContainer AbilityTags = GetAbilityTags(AscendGameplayAbility);

    if (UObject* SourceObject = AscendGameplayAbility->GetAbilitySourceObject())
    {
        const FName SourceSection = GetSectionNameFromObject(SourceObject, AbilityTags);
        if (SourceSection != NAME_None)
        {
            return SourceSection;
        }
    }

    return GetSectionNameFromObject(AscendGameplayAbility, AbilityTags);
}

float UAscendAnimationProvider::GetAbilityPlayRate(const UAscendGameplayAbility* AscendGameplayAbility)
{
    if (!IsValid(AscendGameplayAbility))
    {
        return 0.f;
    }

    const FGameplayTagContainer AbilityTags = GetAbilityTags(AscendGameplayAbility);

    if (UObject* SourceObject = AscendGameplayAbility->GetAbilitySourceObject())
    {
        const float SourcePlayRate = GetPlayRateFromObject(SourceObject, AbilityTags);
        if (SourcePlayRate > 0.0f)
        {
            return SourcePlayRate;
        }
    }

    return GetPlayRateFromObject(AscendGameplayAbility, AbilityTags);
}

FGameplayTagContainer UAscendAnimationProvider::GetAbilityTags(const UAscendGameplayAbility* AscendGameplayAbility)
{
    FGameplayTagContainer AbilityTags = FGameplayTagContainer::EmptyContainer;
    if (!IsValid(AscendGameplayAbility))
    {
        return AbilityTags;
    }

    // Aggregate definition tags, asset tags, and animation context tags for provider lookups.
    AbilityTags.AppendTags(AscendGameplayAbility->GetAbilityDefinitionTags());
    AbilityTags.AppendTags(AscendGameplayAbility->GetAssetTags());

    if (UObject* SourceObject = AscendGameplayAbility->GetAbilitySourceObject())
    {
        AppendAnimationContextTagsFromObject(SourceObject, AbilityTags);
    }

    AppendAnimationContextTagsFromObject(AscendGameplayAbility, AbilityTags);

    return AbilityTags;
}

UAnimMontage* UAscendAnimationProvider::GetAnimationMontageFromObject(const UObject* CandidateObject, const FGameplayTagContainer& AbilityTags)
{
    if (IsValid(CandidateObject) && CandidateObject->Implements<UAnimationMontageProviderInterface>())
    {
        return IAnimationMontageProviderInterface::Execute_GetAnimationMontage(CandidateObject, AbilityTags);
    }

    return nullptr;
}

FName UAscendAnimationProvider::GetSectionNameFromObject(const UObject* CandidateObject, const FGameplayTagContainer& AbilityTags)
{
    if (IsValid(CandidateObject) && CandidateObject->Implements<UAnimationMontageProviderInterface>())
    {
        return IAnimationMontageProviderInterface::Execute_GetSectionName(CandidateObject, AbilityTags);
    }

    return NAME_None;
}

float UAscendAnimationProvider::GetPlayRateFromObject(const UObject* CandidateObject, const FGameplayTagContainer& AbilityTags)
{
    if (IsValid(CandidateObject) && CandidateObject->Implements<UAnimationMontageProviderInterface>())
    {
        return IAnimationMontageProviderInterface::Execute_GetPlayRate(CandidateObject, AbilityTags);
    }

    return 0.0f;
}

void UAscendAnimationProvider::AppendAnimationContextTagsFromObject(const UObject* CandidateObject, FGameplayTagContainer& InOutTags)
{
    if (IsValid(CandidateObject) && CandidateObject->Implements<UAnimationContextProviderInterface>())
    {
        FGameplayTagContainer ContextTags;
        if (IAnimationContextProviderInterface::Execute_GetAnimationContext(CandidateObject, ContextTags) && ContextTags.IsValid())
        {
            InOutTags.AppendTags(ContextTags);
        }
    }
}
