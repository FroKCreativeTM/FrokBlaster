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

	// 나중에 실 개발때 바꿀 옵션이다.
	bUseControllerRotationYaw = false;

	// 움직이는 방향으로 회전한다.
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);	// DOREPLIFETIME 없이 복제가 가능하다.

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// 메시, 캡슐이 카메라와 부딛히는 경우 무시하게 만들어준다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// 회전 비율을 설정한다.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	// 제자리에서 돌고 있지 않다.
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// 여기서도 서버에서 돌아가는 업데이트 프리퀀시를 적용할 수 있다.
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

	// 먼저 HUD를 업데이트한다.
	UpdateHUDHealth();
	if (HasAuthority())
	{
		// 서버의 경우 ApplyDamage가 날아온 경우 ReceiveDamage를 실행한다.
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		// 시뮬 프록시보다 더 높은 권한을 가진 경우
		// 즉 서버이거나 클라이언트여도 내가 직접 컨트롤하는 경우
		AimOffset(DeltaTime);
	}
	else
	{
		// 시뮬 프록시보다 이하의 권한을 가진 경우
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

	// 여기에 복제될 변수를 등록하는 매크로를 적는다. (DOREPLIFETIME, 조건이 있다면 DOREPLIFETIME_CONDITION)
	// COND_OwnerOnly : 만약 클라이언트 중 하나(autonomous proxy)와 Overlap된 경우
	// overlapping된 무기는 단지 그 캐릭터를 소유한 클라이언트한테만 복제가 된다.
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
	// 무기를 안 들고 있다면 당연히 실행X.
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	// 메시에서 애님인스턴스를 가져온다.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		// 몽타주를 가져온 뒤
		// 맞는 섹션의 애니메이션을 실행한다.
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	// 무기를 안 들고 있다면 당연히 실행X.
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	// 메시에서 애님인스턴스를 가져온다.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		// 몽타주를 가져온 뒤
		// 맞는 섹션의 애니메이션을 실행한다.
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	// 죽었을 경우 그에 맞는 몽타주를 실행한다.
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
	// 데미지를 받았을 경우 
	// Clamp(0.f~MaxHealth 사이로)를 이용해서 간격 사이로 값을 조절한다.
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	// 업데이트된 Health 값을 기반으로 체력 HUD를 업데이트 한다.
	UpdateHUDHealth();

	// 맞았을 경우 실행하는 애니메이션 몽타주를 실행한다.
	PlayHitReactMontage();

	// 만약 체력이 0이라면
	if (Health == 0.f)
	{
		// 현재 게임 컨텍스트를 관리중인 게임 모드를 가져온다.
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);

			// 플레이어를 제거한다.
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	// 플레이어 컨트롤러의 유무를 파악한다.
	BlasterPlayerController 
		= BlasterPlayerController == nullptr ? 
		Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		// 플레이어 컨트롤러를 통해서 Health값을 업데이트한다.
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// 회전하고 있는 Yaw 방향이 어딘가에 따라서 도는 방향을 저장한다.
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	// 만약 돌고 있는 상황이면
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// 스피드(4.f)에 따른 Aim Offset yaw값을 보간한다.
		// Interpolate float from Current to Target. Scaled by distance to Target, 
		// so it has a strong start speed and ease out.
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		// 보간된 값을 적용한다.
		AO_Yaw = InterpAO_Yaw;
		// 절대값이 15도보다 작은 경우
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			// 돌지 않고 있다고 판단한다
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			// 원래 Yaw값을 시작값으로 설정한다.
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

// 체력 프로퍼티 값이 업데이트 되면 불려지는 함수
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

	// 이것이 없다면 서버 역할을 맡는 입장에서는 위젯이 작동하지 않는다.
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
	// 장착은 서버에서 타당성(Validation)을 파악한 뒤 처리해야 한다.
	// 문제는 이러면 클라이언트는 잡질 못함(바꿀 필요 있음)
	if (Combat)
	{
		// 서버라면 바로 실행이 가능하다.
		if (HasAuthority())
			Combat->EquipWeapon(OverlappingWeapon);
		// 클라이언트라면 서버에게 대리로 맡겨서 결과를 받아야한다.
		else
			ServerEquipButtonPressed();
	}
}

// RPC 함수는 뒤에 _Implementation이 붙어야한다.
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	// 서버에서 실행한다면 당연히 HasAuthority()를 true 반환할 것이다.
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
	// 무기가 없다면 딱히 적용할 필요가 없다.	
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();	// 현재 떨어지고 있는가를 구한다.

	if (Speed == 0.f && !bIsInAir) // 점프하지 않은 단순히 서있는 상태라면
	{
		// 현재 회전중이다.
		bRotateRootBone = true;
		// 현재 Aim Rotation을 Yaw값을 이용해서 구한다.
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// 이후 현재 Aim과 시작 Aim의 차이값을 정규화 한다.
		FRotator DeltaAimRotation 
			= UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		// 정규화된 Aim Yaw 차이값을 저장한다.
		AO_Yaw = DeltaAimRotation.Yaw;		
		
		// 만약 돌고 있지 않는 경우
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		// 현재 도는 방향을 저장한다. (Aim offset 가지는 상황이라 저장이 가능하다.)
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // 움직이는 상태(걷기, 뛰기, 점프)
	{
		// 루트 본이 회전중이 아니다.
		bRotateRootBone = false;
		// 시작 Aim의 회전값을 현재 폰의 Yaw값으로 둔다. 
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// Aimoff셋을 0으로 둔다.(짜피 오프셋이 필요한 애니메이션을 실행하지 않으니까!)
		AO_Yaw = 0.f;
		// Yaw회전을 할 수 있게 설정한다.
		bUseControllerRotationYaw = true;
		// 현재 회전중이라 제자리 회전이 아니다.
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
	// 무기가 없다면 딱히 적용할 필요가 없다.
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
	// 만약 전투 시스템이 없다면 반환
	if (Combat == nullptr) return nullptr;

	// 현재 캐릭터가 가지고 있는 전투 시스템에 등록된 무기를 반환한다.
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}
