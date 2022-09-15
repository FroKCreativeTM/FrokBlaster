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

	// 몽타주 함수들
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	// 데미지를 받았을 경우를 위한 함수
	// ProjectileBullet의 OnHit 함수에 있는
	// ApplyDamage와 연동되는 함수이다.
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, 
		float Damage, const UDamageType* DamageType, 
		class AController* InstigatorController, 
		AActor* DamageCauser);
	void UpdateHUDHealth();

	virtual void OnRep_ReplicatedMovement() override;

	// 죽었을 경우 멀티캐스팅으로 뿌려진다.
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
	// 클라이언트에서 시뮬되고 있는 프록시들의 turning을 좀 더 매끄럽게 하기 위한 함수
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

	// 발사 모션 몽타주
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	// 맞았을 경우 몽타주
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	// 죽었을 경우 몽타주
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

	// 플레이어 상태 관련 변수 및 함수
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	// HUD 등을 관리할 현재 캐릭터의 컨트롤러 클래스
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	/**
	* 죽을 때 Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// 런타임에 바뀌는 인스턴스
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// 블루프린트에서 사용하는 머터리얼 인스턴스
	// 동적 머터리얼 인스턴스와 사용한다.
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* 봇 죽는 기능(위에서 효과)
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
	// 적 인스턴스 또한 우리가회전하고 있는가를 알게한다.
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	// 제거 여부 파악용
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
};