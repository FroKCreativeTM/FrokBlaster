#include "CombatComponent.h"
#include "FrokBlaster/Weapon/Weapon.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


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
	EquippedWeapon->ShowPickupWidget(false);
}
