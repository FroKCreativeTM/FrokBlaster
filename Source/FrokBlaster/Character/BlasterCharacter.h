// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FrokBlaster/BlasterTypes/TurningInPlace.h"
#include "FrokBlaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class FROKBLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();

	// �¾��� ��츦 ������ ��Ƽĳ�����Ѵ�(�̰� ������ Hit Reaction�� ���������� �Ͼ��.)
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	virtual void OnRep_ReplicatedMovement() override;

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

	void CalculateAO_Pitch();
	// Ŭ���̾�Ʈ���� �ùĵǰ� �ִ� ���Ͻõ��� turning�� �� �� �Ų����� �ϱ� ���� �Լ�
	void SimProxiesTurn();	

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

	// ������ ��������� ����� ����
	// ReplicatedUsing 
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	// �޼��� �̸� �տ� Server�� �ٰ�
	// UFUNCTION(Server) ��ũ�θ� ����Ѵٸ� RPC �޼����̴�.
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// ��� �������� ���� �� �����ϱ� ���� ����
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	// �߻� ��� ��Ÿ��
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	// �¾��� ��� ��Ÿ��
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;


	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	// �÷��̾� ���� ���� ���� �� �Լ�
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

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
	// �� �ν��Ͻ� ���� �츮��ȸ���ϰ� �ִ°��� �˰��Ѵ�.
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
};