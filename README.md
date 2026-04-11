# Project Ascend

> **Unreal Engine 5.5** · **Top-Down** · **Gameplay Ability System**

Project Ascend is a small top-down action RPG gameplay framework built on Unreal Engine 5.5. The project is centered around a clean GAS setup, a modular ability pipeline, and a Leap Slam implementation that can grow into a larger combat sandbox without needing to rework the foundation.

The current content is intentionally small, but the systems are organized so that new abilities, new animation sources, and new targeting or damage rules can be added without turning the codebase into a one-off ability prototype.

---

## Table of Contents

- [What Is Built](#what-is-built)
- [Project Overview](#project-overview)
- [Systems](#systems)
  - [GAS Foundation](#gas-foundation)
  - [Attributes and Damage Flow](#attributes-and-damage-flow)
  - [Ability Definitions and Fragments](#ability-definitions-and-fragments)
  - [Leap Slam](#leap-slam)
  - [Input and Ability Slots](#input-and-ability-slots)
  - [Animation Layer](#animation-layer)
  - [Characters and Ownership](#characters-and-ownership)
  - [Targeting and Effect Containers](#targeting-and-effect-containers)
  - [UI Bridge](#ui-bridge)
- [Why the Project Is Structured This Way](#why-the-project-is-structured-this-way)
- [Extending the Project](#extending-the-project)
- [Project Structure](#project-structure)

---

## What Is Built

- A **GAS-based gameplay foundation** with a custom Ability System Component
- A core **attribute set** built around Health, Mana, AttackSpeed, Damage, and meta attributes
- A fully implemented **Leap Slam** ability with montage-driven timing, root motion jump arc, landing effects, and AttackSpeed scaling
- A **definition-driven ability setup** using DataAssets and fragments for configuration
- A **targeting and effect container pipeline** for applying effects in a reusable way
- A **runtime input and slot system** for mapping granted abilities to input tags
- An **animation provider layer** that can resolve montages from different sources without hardcoding them into each ability
- A simple **enemy character flow** with damage handling and death behavior
- A lightweight **UI bridge** that listens to the ASC and forwards attribute changes to Blueprint widgets

---

## Project Overview

At a glance, the project aims to keep gameplay code readable while still leaving room to grow.

The ability content is intentionally focused, but the surrounding systems are not hard-wired around a single skill. Abilities can be configured through definitions, effects are applied through reusable containers, and animation selection is separated from ability logic so the same gameplay class can evolve alongside different weapons, stances, or future variants.

That balance matters here: the project should stay understandable today, but it should also avoid the kind of shortcuts that make the second or third ability more expensive than the first.

---

## Systems

### GAS Foundation

<details>
<summary><strong>Custom Ability System Component and gameplay flow</strong></summary>

The project uses a custom `UAscendAbilitySystemComponent` as the main runtime hub.

It keeps track of granted ability definitions, input state, slot assignments, and semantic tags attached to each granted ability. In practice, this means the ASC is doing more than simply hosting GAS abilities; it is also acting as the place where ability metadata and runtime routing meet.

A custom asset manager initializes GAS global data early in startup. This keeps targeting and cue-related systems reliable and avoids the usual “works in one setup, breaks in another” kind of initialization issue.

The goal here is not to add extra layers for their own sake. It is to give the project one place where ability-related runtime state can live cleanly.

</details>

### Attributes and Damage Flow

<details>
<summary><strong>Health, Mana, AttackSpeed, Damage, and meta attributes</strong></summary>

The main attribute set contains the expected gameplay values for this project:

- `Health` / `MaxHealth`
- `Mana` / `MaxMana`
- `AttackSpeed`
- `Damage`
- `IncomingDamage`
- `Healing`

`AttackSpeed` is especially important because it feeds directly into ability pacing. In Leap Slam, it affects how quickly the move is performed rather than sitting as a disconnected stat.

Damage is handled through a small pipeline instead of directly subtracting health inside each ability. The execution calculation resolves outgoing damage, writes it into the `IncomingDamage` meta attribute, and the attribute set applies the final result in `PostGameplayEffectExecute`.

That extra step keeps the damage path centralized. It makes later additions like resistances, armor, conditional multipliers, or special reactions much easier to add without rewriting every damaging ability.

</details>

### Ability Definitions and Fragments

<details>
<summary><strong>Data-driven ability configuration</strong></summary>

Abilities are configured through `UAscendAbilityDefinition` and related fragment types rather than placing every configurable field directly on the gameplay ability base class.

The fragments currently cover things like:

- input binding
- effect containers
- animation provider data

This keeps cross-cutting configuration in one place while letting the ability class focus on behavior.

In other words:

- the **definition** answers “what is this ability wired to?”
- the **ability class** answers “how does this ability behave?”

That split helps once a project starts adding variations. A future ability does not have to copy unrelated properties just because another ability needed them.

</details>

### Leap Slam

<details>
<summary><strong>Montage-driven leap with arc travel and landing effects</strong></summary>

Leap Slam is built as a montage-driven GAS ability.

At activation, the ability resolves a target location, caches it, and begins its animation flow. A gameplay event sent from an animation notify is used as the transition point between wind-up and actual movement. When that moment arrives, the ability starts a root motion jump task and moves the character along an arc instead of teleporting.

On landing, the ability applies its landing effects and damage, then hands control back to the montage for the final section.

A few details make the move feel more deliberate:

- the target is captured up front instead of drifting with the cursor during wind-up
- travel is handled with root motion jump force rather than an instant relocation
- landing damage can scale with travel distance
- AttackSpeed changes both the timing of the animation and the travel pacing

The result is still a focused single ability, but it is implemented in a way that can be reused as a pattern for other montage-based movement skills.

</details>

### Input and Ability Slots

<details>
<summary><strong>Buffered input and runtime slot routing</strong></summary>

Input is routed through tags rather than binding gameplay classes directly to keys.

The project also keeps a small runtime slot system. The ASC remains the source of truth, while `UAscendAbilitySlotComponent` exposes player-facing slot operations such as assigning, moving, or swapping abilities between slot tags.

This is useful for two reasons:

1. input routing stays flexible
2. gameplay ability data does not have to know about concrete keyboard layout decisions

The input path is buffered and flushed in a controlled phase instead of trying to activate everything directly inside the raw input callback. That gives the project a steadier ability activation flow and leaves room for input-hold or confirm/cancel style behavior.

</details>

### Animation Layer

<details>
<summary><strong>Why animation is separated from the ability itself</strong></summary>

The animation layer is one of the most important parts of the project’s extensibility.

`UAscendAnimationAbility` handles montage playback, lifecycle callbacks, and gameplay-event listening. The ability itself does not need to own every animation reference directly. Instead, animation playback data can be resolved through an `UAscendAnimationProvider`.

That provider follows a simple chain:

1. ask the ability’s **source object**
2. if nothing is provided there, ask the **ability itself**
3. if nothing is provided there either, use the **provider defaults**

This is what allows the same gameplay ability to stay stable while animation sources change around it.

</details>

<details>
<summary><strong><code>UAnimationMontageProviderInterface</code> — what it does and why it exists</strong></summary>

This interface is responsible for answering a very practical question:

**“Given this ability context, which montage, section, and play rate should be used?”**

Objects that drive animation decisions can implement it. That could be a weapon actor, a stance-related component, or even the ability itself.

The main benefit is that animation ownership can live where it naturally belongs.

For example, if the project later adds weapon-specific attacks, the gameplay ability does not need separate hardcoded montage references for sword, axe, spear, and so on. The weapon can provide the montage instead. The ability remains responsible for gameplay; the source object becomes responsible for animation flavor.

That is why this interface matters. It keeps animation selection open-ended without making the ability class heavier every time a new variant appears.

</details>

<details>
<summary><strong><code>UAnimationContextProviderInterface</code> — what it adds on top</strong></summary>

If the montage provider answers **“which animation should play?”**, the context provider answers:

**“What extra information should be considered before choosing that animation?”**

This interface can contribute context tags and related data before montage resolution happens.

A future weapon could add tags like:

- `Weapon.Type.GreatSword`
- `Weapon.Weight.Heavy`

An ability could add temporary context like:

- combo step
- charged state
- alternate stance

Those tags are collected and passed into the montage query. This lets animation decisions become richer without turning every ability into a big web of `if` branches.

The interface also exposes hooks for effect context and last event magnitude. Those are not the center of the current content, but they are useful extension points for animation-driven combat setups where context needs to move together with montage selection.

In short, the montage provider chooses the asset, and the context provider helps describe the situation.

</details>

<details>
<summary><strong>Custom montage task</strong></summary>

The project includes a custom `UAbilityTask_PlayMontageWaitEvent` task.

Its role is straightforward: play a montage, listen for gameplay events, and react to completion, blend-out, interruption, or cancellation in one place. It also supports matching against tag containers instead of relying on a single event tag.

For a montage-driven ability like Leap Slam, this makes the flow easier to follow and easier to reuse.

</details>

### Characters and Ownership

<details>
<summary><strong>Player, enemy, and ASC ownership</strong></summary>

The player and enemy do not share exactly the same ownership pattern.

The player path uses a PlayerState-owned ASC, which is a healthy default when a character may later need persistence across pawn changes, death, or respawn-related flows.

The enemy path keeps the ASC directly on the character, which keeps that setup simple and self-contained.

This is less about overbuilding and more about choosing sensible ownership for two different actor roles.

</details>

### Targeting and Effect Containers

<details>
<summary><strong>Reusable effect application</strong></summary>

The effect container system separates three things:

- what effect classes will be applied
- how targets are found
- when the ability chooses to trigger them

Targeting is represented through dedicated target type structs, and the ability can build a runtime spec from configured data when needed.

That means a gameplay ability is not forced to manually assemble every effect path inline. The ability decides when something happens, while the container and target type decide what gets applied and to whom.

This keeps targeting logic reusable across multiple abilities and helps avoid one-off effect code.

</details>

### UI Bridge

<details>
<summary><strong>Direct ASC-to-widget binding</strong></summary>

The UI layer stays intentionally light.

`UAscendAbilitySystemWidget` binds directly to the ASC, listens for attribute changes and slot changes, and forwards those updates to Blueprint events. That keeps the runtime HUD bridge simple and easy to follow.

There is no ViewModel-focused explanation here because this project does not need one to explain what is currently in the source. The UI section exists only to describe the actual runtime bridge that is present.

</details>

---

## Why the Project Is Structured This Way

A project like this can easily fall into one of two traps:

- making everything so simple that the second ability becomes messy
- making everything so abstract that the first ability becomes hard to understand

The structure here tries to stay in the middle.

A few decisions reflect that:

- **Definitions and fragments** keep repeated configuration out of the ability class hierarchy
- **Animation providers and interfaces** let animation logic evolve without bloating gameplay logic
- **Effect containers and target types** keep reusable combat plumbing out of individual abilities
- **A custom ASC** provides one stable place for input, slotting, and ability metadata

The project is meant to be extendable, but it is also meant to stay readable.

---

## Extending the Project

A few examples of how the current structure can grow without major rewrites:

<details>
<summary><strong>Adding a new ability</strong></summary>

Create a new gameplay ability class only when the behavior is genuinely new, then configure it through a definition and fragments.

If the ability shares an existing behavior pattern, most of the work can stay at the configuration level.

</details>

<details>
<summary><strong>Adding weapon-dependent animations</strong></summary>

Implement `UAnimationMontageProviderInterface` on the weapon or source object, and optionally `UAnimationContextProviderInterface` if that object needs to enrich the animation query.

That allows the same gameplay ability to play different montages depending on the equipped item without rewriting the ability logic.

</details>

<details>
<summary><strong>Adding new targeting styles</strong></summary>

Add a new target type struct and plug it into effect containers.

This keeps targeting variations out of the ability body and makes them reusable across multiple skills.

</details>

<details>
<summary><strong>Adding combat rules</strong></summary>

Because damage already flows through an execution and meta-attribute path, new combat rules such as armor, resistances, conditional modifiers, or special reactions have a natural place to live.

</details>

---

## Project Structure

```text
Source/ProjectAscend/
├── AbilitySystem/
│   ├── Abilities/
│   ├── AbilityTasks/
│   ├── Attributes/
│   ├── Calculations/
│   ├── Data/
│   ├── Definition/
│   ├── Effects/
│   ├── Fragments/
│   ├── Providers/
│   └── Targeting/
├── Animation/
│   ├── Notifies/
│   └── NotifyStates/
├── Character/
│   ├── Base/
│   ├── Enemy/
│   └── Player/
├── Core/
├── Input/
├── Interfaces/
│   └── Animation/
├── Player/
├── Settings/
└── UI/
```

---

If you want to read the project from top to bottom, a good starting path is:

1. `AscendAbilitySystemComponent`
2. `AscendGameplayAbility` / `AscendAnimationAbility`
3. `AscendLeapSlamAbility`
4. `AscendAnimationProvider` and the animation interfaces
5. `AscendAbilityDefinition` and ability fragments

That path gives a clear picture of how the project is wired without having to jump through every file first.
