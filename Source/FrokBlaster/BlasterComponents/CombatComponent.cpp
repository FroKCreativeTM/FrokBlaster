#include "CombatComponent.h"
#include "FrokBlaster/Weapon/Weapon.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"

const float TRACE_LENGTH = 80000.f;

UCombatComponent::UCombatComponent()
{
	// ���� Ʈ���̽��� ���ؼ� Tick�� Ȱ��ȭ�Ѵ�.
	PrimaryComponentTick.bCanEverTick = true;

	// �Ϲ����� ����
	// ���� �� ���ǵ��� ���̸� �д�.
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		// ���ؼ� ��ġ�� �����ϴ� �Լ�
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	// ���� �ÿ� �Ϲ����� ����� ���ǵ带 �ٸ��� �����Ѵ�.
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// �������� ó���ϱ� ���� �Լ�
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	// ���� �ÿ� �Ϲ����� ����� ���ǵ带 �ٸ��� �����Ѵ�.
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;

	// ������ ����ִٸ�
	if (GEngine && GEngine->GameViewport)
	{
		// ����Ʈ�� ����� �����´�.
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ȭ���� �߾��� ����Ų��.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// 2D ȭ���� ��ǥ�� 3D�� ���� ��ǥ�� ��ȯ�Ѵ�.
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// ���� �÷��̾� ��Ʈ�ѷ��� �ִ� ��ġ�� ����Ʈ�� ����� �̿��ؼ�
	// 3���� �������� �÷��̾��� ��ġ�� �÷��̾ �ٶ󺸴� Direction�� ���Ѵ�.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	// ��ǥ ��ȯ�� �����ߴٸ�
	if (bScreenToWorld)
	{
		// ���� Ʈ���̽��� �����Ѵ�.
		// �� �� End ��ġ�� Start ��ġ���� ����������κ��� ����� �� ������ Ʈ���̽��� �����ϰ�
		// �밭 100000���� ����־���.
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// ���� ���̴� �繰(����)�� ã�� ������ ���������� Ʈ���̽��Ѵ�.
		// Ʈ���̽� ����� TraceHitResult�� ����ȴ�.
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
		if (TraceHitResult.GetActor() 
			&& TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr 
		|| Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller)
		: Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		// ���� �����Ѵ�...
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			// ���ؼ� ������� ����Ѵ�.
			// ��ũ�� ���� �ƴ� ����� ���ǵ尡 �ٸ��� �̸� ���� �����Ѵ�.
			if (Character->bIsCrouched) 
			{
				// [0, MaxWalkSpeedCrouched] -> [0, 0.75]�� ����ȭ�Ѵ�.
				FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeedCrouched);
				FVector2D VelocityMultiplierRange(0.f, 0.75f);
				FVector Velocity = Character->GetVelocity();
				Velocity.Z = 0.f;	// Z�� �Ű澵 �ʿ䰡 ����

				// ���� �ȴ� �ӵ��� ���� ����ȭ ���ฦ ������ �� �����Ѵ�.
				CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,
					VelocityMultiplierRange, Velocity.Size());
			}
			else
			{
				// [0, MaxWalkSpeed] -> [0, 1]�� ����ȭ�Ѵ�.
				FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
				FVector2D VelocityMultiplierRange(0.f, 1.f);
				FVector Velocity = Character->GetVelocity();
				Velocity.Z = 0.f;	// Z�� �Ű澵 �ʿ䰡 ����

				// ���� �ȴ� �ӵ��� ���� ����ȭ ���ฦ ������ �� �����Ѵ�.
				CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,
					VelocityMultiplierRange, Velocity.Size());
			}

			if (Character->GetCharacterMovement()->IsFalling())
			{
				// ���� �������� ���̶��
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 
					2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor,
					0.f, DeltaTime, 2.25);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}


}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	// �������� ó���ϵ���! ������ �̷��� ���������� �۵��Ѵ�.
	// Ŭ���̾�Ʈ���� Notify�� �ʿ��ϴ�. (�� Multicast�� �ʿ��ϴ�.)
	if (bFireButtonPressed)
	{
		// HitResult�� TraceUnderCrosshairs �Լ����� ������� ������ �� 
		// ���� ���� ��ġ�� ServerFire���� �׸��� MulticastFire�� ���޵ȴ�.
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = .75f;
		}
	}
}

// RPC �Լ��� _Implementation�� �ٴ´�!!!!
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// ���������� Fire�� �ൿ�� multicast�Ѵ�.
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// Fire�� ������ ��� ���� ������ ������ ó���Ѵ�.
	// �� Fire�� ���� ������ Server�� ó�����ִ� ���� ���̴�.
	// �׸��� �� ����� multicast�Ѵ�. (�� ��� Ŭ���̾�Ʈ���� �����Ѵ�.)
	if (EquippedWeapon == nullptr) return;

	if (Character)
	{
		Character->PlayFireMontage(bAiming);	// ĳ���Ͱ� �߻��ϴ� ����� ���ϰ� �Ѵ�.
		EquippedWeapon->Fire(TraceHitTarget);	// ���ÿ� �������� ������ �߻� �ִϸ��̼��� �����Ų��.
												// �� �� ����� ���� ��ġ�� ���� �����Ѵ�.
	}
}
 