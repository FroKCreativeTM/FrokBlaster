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

	// 서버에서 쓰는 함수는 UFUNCTION을 통해서 알려줘야 한다.
	// 문제는 Server는 단순히 Server에서만 처리하는 것!
	// FVector_NetQuantize는 네트워킹을 위해서 사용되는 직렬화된 자료형이다.
	// 0 decimal place of precision. Up to 20 bits per component. 
	// Valid range: 2^20 = +/- 1,048,576
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	// NetMulticast : 멀티캐스트할 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// HUD의 조준선을 설정한다.
	void SetHUDCrosshairs(float DeltaTime);

private :
	class ABlasterCharacter*		Character;	// 장착중인 캐릭터
	class ABlasterPlayerController* Controller;	// 조종중인 플레이어 컨트롤러
	class ABlasterHUD*				HUD;		// 조종중인 플레이어 컨트롤러

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;		// 장착중인 무기

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
	 * 조준선과 HUD
	 */
	float CrosshairVelocityFactor;	// 정규화된 걷기 속도
	float CrosshairInAirFactor;	// 정규화된 걷기 속도

public : 
	// ABlasterCharacter 입장에서는 이 컴포넌트의 모든 것에 접근할 수 있어야 한다.
	friend class ABlasterCharacter;
};
