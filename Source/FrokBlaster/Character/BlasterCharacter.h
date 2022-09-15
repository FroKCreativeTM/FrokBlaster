// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FrokBlaster/BlasterTypes/TurningInPlace.h"
#include "FrokBlaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
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

	// ��Ÿ�� �Լ���
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	// �������� �޾��� ��츦 ���� �Լ�
	// ProjectileBullet�� OnHit �Լ��� �ִ�
	// ApplyDamage�� �����Ǵ� �Լ��̴�.
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, 
		float Damage, const UDamageType* DamageType, 
		class AController* InstigatorController, 
		AActor* DamageCauser);
	void UpdateHUDHealth();

	virtual void OnRep_ReplicatedMovement() override;

	// �׾��� ��� ��Ƽĳ�������� �ѷ�����.
	void Elim();

	// 
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	virtual void Destroyed() override;

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

	// �׾��� ��� ��Ÿ��
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

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

	// HUD ���� ������ ���� ĳ������ ��Ʈ�ѷ� Ŭ����
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	/**
	* ���� �� Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// ��Ÿ�ӿ� �ٲ�� �ν��Ͻ�
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// �������Ʈ���� ����ϴ� ���͸��� �ν��Ͻ�
	// ���� ���͸��� �ν��Ͻ��� ����Ѵ�.
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* �� �״� ���(������ ȿ��)
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

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
	// ���� ���� �ľǿ�
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
};