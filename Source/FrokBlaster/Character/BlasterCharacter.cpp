#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "FrokBlaster/Weapon/Weapon.h"
#include "FrokBlaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "FrokBlaster/FrokBlaster.h"
#include "FrokBlaster/HUD/CharacterOverlay.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"
#include "FrokBlaster/GameMode/BlasterGameMode.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ���߿� �� ���߶� �ٲ� �ɼ��̴�.
	bUseControllerRotationYaw = false;

	// �����̴� �������� ȸ���Ѵ�.
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);	// DOREPLIFETIME ���� ������ �����ϴ�.

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// �޽�, ĸ���� ī�޶�� �ε����� ��� �����ϰ� ������ش�.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// ȸ�� ������ �����Ѵ�.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	// ���ڸ����� ���� ���� �ʴ�.
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// ���⼭�� �������� ���ư��� ������Ʈ �������ø� ������ �� �ִ�.
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// ���� HUD�� ������Ʈ�Ѵ�.
	UpdateHUDHealth();
	if (HasAuthority())
	{
		// ������ ��� ApplyDamage�� ���ƿ� ��� ReceiveDamage�� �����Ѵ�.
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		// �ù� ���Ͻú��� �� ���� ������ ���� ���
		// �� �����̰ų� Ŭ���̾�Ʈ���� ���� ���� ��Ʈ���ϴ� ���
		AimOffset(DeltaTime);
	}
	else
	{
		// �ù� ���Ͻú��� ������ ������ ���� ���
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}

	HideCameraIfCharacterClose();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ���⿡ ������ ������ ����ϴ� ��ũ�θ� ���´�. (DOREPLIFETIME, ������ �ִٸ� DOREPLIFETIME_CONDITION)
	// COND_OwnerOnly : ���� Ŭ���̾�Ʈ �� �ϳ�(autonomous proxy)�� Overlap�� ���
	// overlapping�� ����� ���� �� ĳ���͸� ������ Ŭ���̾�Ʈ���׸� ������ �ȴ�.
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	// ���⸦ �� ��� �ִٸ� �翬�� ����X.
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	// �޽ÿ��� �ִ��ν��Ͻ��� �����´�.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		// ��Ÿ�ָ� ������ ��
		// �´� ������ �ִϸ��̼��� �����Ѵ�.
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	// ���⸦ �� ��� �ִٸ� �翬�� ����X.
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	// �޽ÿ��� �ִ��ν��Ͻ��� �����´�.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		// ��Ÿ�ָ� ������ ��
		// �´� ������ �ִϸ��̼��� �����Ѵ�.
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	// �׾��� ��� �׿� �´� ��Ÿ�ָ� �����Ѵ�.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, 
	float Damage, const UDamageType* DamageType, 
	AController* InstigatorController, 
	AActor* DamageCauser)
{
	// �������� �޾��� ��� 
	// Clamp(0.f~MaxHealth ���̷�)�� �̿��ؼ� ���� ���̷� ���� �����Ѵ�.
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	// ������Ʈ�� Health ���� ������� ü�� HUD�� ������Ʈ �Ѵ�.
	UpdateHUDHealth();

	// �¾��� ��� �����ϴ� �ִϸ��̼� ��Ÿ�ָ� �����Ѵ�.
	PlayHitReactMontage();

	// ���� ü���� 0�̶��
	if (Health == 0.f)
	{
		// ���� ���� ���ؽ�Ʈ�� �������� ���� ��带 �����´�.
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);

			// �÷��̾ �����Ѵ�.
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	// �÷��̾� ��Ʈ�ѷ��� ������ �ľ��Ѵ�.
	BlasterPlayerController 
		= BlasterPlayerController == nullptr ? 
		Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		// �÷��̾� ��Ʈ�ѷ��� ���ؼ� Health���� ������Ʈ�Ѵ�.
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// ȸ���ϰ� �ִ� Yaw ������ ��򰡿� ���� ���� ������ �����Ѵ�.
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	// ���� ���� �ִ� ��Ȳ�̸�
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// ���ǵ�(4.f)�� ���� Aim Offset yaw���� �����Ѵ�.
		// Interpolate float from Current to Target. Scaled by distance to Target, 
		// so it has a strong start speed and ease out.
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		// ������ ���� �����Ѵ�.
		AO_Yaw = InterpAO_Yaw;
		// ���밪�� 15������ ���� ���
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			// ���� �ʰ� �ִٰ� �Ǵ��Ѵ�
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			// ���� Yaw���� ���۰����� �����Ѵ�.
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

//void ABlasterCharacter::MulticastHit_Implementation()
//{
//	PlayHitReactMontage();
//}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

// ü�� ������Ƽ ���� ������Ʈ �Ǹ� �ҷ����� �Լ�
void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	// �̰��� ���ٸ� ���� ������ �ô� ���忡���� ������ �۵����� �ʴ´�.
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABlasterCharacter::LookUp);
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	// ������ �������� Ÿ�缺(Validation)�� �ľ��� �� ó���ؾ� �Ѵ�.
	// ������ �̷��� Ŭ���̾�Ʈ�� ���� ����(�ٲ� �ʿ� ����)
	if (Combat)
	{
		// ������� �ٷ� ������ �����ϴ�.
		if (HasAuthority())
			Combat->EquipWeapon(OverlappingWeapon);
		// Ŭ���̾�Ʈ��� �������� �븮�� �ðܼ� ����� �޾ƾ��Ѵ�.
		else
			ServerEquipButtonPressed();
	}
}

// RPC �Լ��� �ڿ� _Implementation�� �پ���Ѵ�.
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	// �������� �����Ѵٸ� �翬�� HasAuthority()�� true ��ȯ�� ���̴�.
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}
void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// ���Ⱑ ���ٸ� ���� ������ �ʿ䰡 ����.	
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();	// ���� �������� �ִ°��� ���Ѵ�.

	if (Speed == 0.f && !bIsInAir) // �������� ���� �ܼ��� ���ִ� ���¶��
	{
		// ���� ȸ�����̴�.
		bRotateRootBone = true;
		// ���� Aim Rotation�� Yaw���� �̿��ؼ� ���Ѵ�.
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// ���� ���� Aim�� ���� Aim�� ���̰��� ����ȭ �Ѵ�.
		FRotator DeltaAimRotation 
			= UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		// ����ȭ�� Aim Yaw ���̰��� �����Ѵ�.
		AO_Yaw = DeltaAimRotation.Yaw;		
		
		// ���� ���� ���� �ʴ� ���
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		// ���� ���� ������ �����Ѵ�. (Aim offset ������ ��Ȳ�̶� ������ �����ϴ�.)
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // �����̴� ����(�ȱ�, �ٱ�, ����)
	{
		// ��Ʈ ���� ȸ������ �ƴϴ�.
		bRotateRootBone = false;
		// ���� Aim�� ȸ������ ���� ���� Yaw������ �д�. 
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// Aimoff���� 0���� �д�.(¥�� �������� �ʿ��� �ִϸ��̼��� �������� �����ϱ�!)
		AO_Yaw = 0.f;
		// Yawȸ���� �� �� �ְ� �����Ѵ�.
		bUseControllerRotationYaw = true;
		// ���� ȸ�����̶� ���ڸ� ȸ���� �ƴϴ�.
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	// ���Ⱑ ���ٸ� ���� ������ �ʿ䰡 ����.
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}

		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	// ���� ���� �ý����� ���ٸ� ��ȯ
	if (Combat == nullptr) return nullptr;

	// ���� ĳ���Ͱ� ������ �ִ� ���� �ý��ۿ� ��ϵ� ���⸦ ��ȯ�Ѵ�.
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}
