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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

	// Getter / Setter
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	// �������� ���� �Լ��� UFUNCTION�� ���ؼ� �˷���� �Ѵ�.
	// ������ Server�� �ܼ��� Server������ ó���ϴ� ��!
	// FVector_NetQuantize�� ��Ʈ��ŷ�� ���ؼ� ���Ǵ� ����ȭ�� �ڷ����̴�.
	// 0 decimal place of precision. Up to 20 bits per component. 
	// Valid range: 2^20 = +/- 1,048,576
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	// NetMulticast : ��Ƽĳ��Ʈ�� �Լ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// HUD�� ���ؼ��� �����Ѵ�.
	void SetHUDCrosshairs(float DeltaTime);

private :
	class ABlasterCharacter*		Character;	// �������� ĳ����
	class ABlasterPlayerController* Controller;	// �������� �÷��̾� ��Ʈ�ѷ�
	class ABlasterHUD*				HUD;		// �������� �÷��̾� ��Ʈ�ѷ�

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;		// �������� ����

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
	 * ���ؼ��� HUD
	 */
	float CrosshairVelocityFactor;	// ����ȭ�� �ȱ� �ӵ�
	float CrosshairInAirFactor;	// ����ȭ�� �ȱ� �ӵ�

public : 
	// ABlasterCharacter ���忡���� �� ������Ʈ�� ��� �Ϳ� ������ �� �־�� �Ѵ�.
	friend class ABlasterCharacter;
};
