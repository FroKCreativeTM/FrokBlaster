// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FROKBLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

private :
	class ABlasterCharacter* Character;	// 장착중인 캐릭터
	AWeapon* EquippedWeapon;		// 장착중인 무기

public : 
	// ABlasterCharacter 입장에서는 이 컴포넌트의 모든 것에 접근할 수 있어야 한다.
	friend class ABlasterCharacter;

	// Getter / Setter

};
