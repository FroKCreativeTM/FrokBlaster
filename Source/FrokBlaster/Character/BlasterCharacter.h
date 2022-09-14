// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FrokBlaster/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class FROKBLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);

protected:
	virtual void BeginPlay() override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void EquipButtonPressed();
	void CrouchButtonPressed();	

	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);

	virtual void Jump() override;

	void FireButtonPressed();
	void FireButtonReleased();

private :
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	// 원본을 기반으로한 비슷한 복제
	// ReplicatedUsing 
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	// 메서드 이름 앞에 Server가 붙고
	// UFUNCTION(Server) 매크로를 사용한다면 RPC 메서드이다.
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// 어느 방향으로 도는 지 저장하기 위한 변수
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

public:
	// Getter/Setter
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();	
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};