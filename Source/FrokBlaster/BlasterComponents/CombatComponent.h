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
	class ABlasterCharacter* Character;	// �������� ĳ����
	AWeapon* EquippedWeapon;		// �������� ����

public : 
	// ABlasterCharacter ���忡���� �� ������Ʈ�� ��� �Ϳ� ������ �� �־�� �Ѵ�.
	friend class ABlasterCharacter;

	// Getter / Setter

};
