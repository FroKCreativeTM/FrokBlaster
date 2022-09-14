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
#include "FrokBlaster/HUD/BlasterHUD.h"

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
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshairs(DeltaTime);
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
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// ���� ���̴� �繰(����)�� ã�� ������ ���������� Ʈ���̽��Ѵ�.
		// Ʈ���̽� ����� TraceHitResult�� ����ȴ�.
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
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
			FHUDPackage HUDPackage;
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
			HUD->SetHUDPackage(HUDPackage);
		}
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
 