#include "CombatComponent.h"
#include "FrokBlaster/Weapon/Weapon.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

const float TRACE_LENGTH = 80000.f;

UCombatComponent::UCombatComponent()
{
	// 라인 트레이싱을 위해서 Tick을 활성화한다.
	PrimaryComponentTick.bCanEverTick = true;

	// 일반적인 경우와
	// 조준 시 스피드의 차이를 둔다.
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

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
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

	// 조준 시와 일반적인 경우의 스피드를 다르게 설정한다.
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// 서버에서 처리하기 위한 함수
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	// 조준 시와 일반적인 경우의 스피드를 다르게 설정한다.
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

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	// 서버에서 처리하도록! 하지만 이러면 서버에서만 작동한다.
	// 클라이언트한테 Notify가 필요하다. (즉 Multicast가 필요하다.)
	if(bFireButtonPressed)
		ServerFire();
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;

	// 엔진이 살아있다면
	if (GEngine && GEngine->GameViewport)
	{
		// 뷰포트의 사이즈를 가져온다.
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 화면의 중앙을 가리킨다.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// 2D 화면의 좌표를 3D의 월드 좌표로 변환한다.
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 현재 플레이어 컨트롤러가 있는 위치와 뷰포트의 사이즈를 이용해서
	// 3차원 공간상의 플레이어의 위치와 플레이어가 바라보는 Direction을 구한다.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	// 좌표 변환이 성공했다면
	if (bScreenToWorld)
	{
		// 라인 트레이스를 진행한다.
		// 이 때 End 위치는 Start 위치에서 진행방향으로부터 충분히 먼 곳까지 트레이싱을 진행하게
		// 대강 100000으로 잡아주었다.
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// 현재 보이는 사물(액터)를 찾을 때까지 지속적으로 트레이싱한다.
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);

		// 무언가 맞지 않았다면
		if (!TraceHitResult.bBlockingHit)
		{
			// 충돌 지점은 대충 End위치(엄청나게 먼 위치)에 잡아둔다.
			TraceHitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
			// 맞은 위치에 구를 그린다.
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red);
		}
	}
}

// RPC 함수는 _Implementation가 붙는다!!!!
void UCombatComponent::ServerFire_Implementation()
{
	// 서버에서는 Fire한 행동을 multicast한다.
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	// Fire의 역할을 어느 정도 서버가 나눠서 처리한다.
	// 즉 Fire의 메인 로직은 Server가 처리해주는 식인 것이다.
	// 그리고 그 결과를 multicast한다. (즉 모든 클라이언트에서 실행한다.)
	if (EquippedWeapon == nullptr) return;

	if (Character)
	{
		Character->PlayFireMontage(bAiming);	// 캐릭터가 발사하는 모션을 취하게 한다.
		EquippedWeapon->Fire(HitTarget);		// 동시에 장착중인 무기의 발사 애니메이션을 재생시킨다.
												// 이 때 저장된 맞은 위치를 같이 전송한다.
	}
}
 