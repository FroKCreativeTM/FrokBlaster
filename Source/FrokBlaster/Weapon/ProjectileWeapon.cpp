#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// �ѱ� �κ��� Socket�� �����´�.
	const USkeletalMeshSocket* MuzzleSocket 
		= GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	// �ѱ��� �����Ѵ�.
	if (MuzzleSocket)
	{
		// �ѱ��� Ʈ������ ������ �����´�.
		FTransform SocketTransform 
			= MuzzleSocket->GetSocketTransform(GetWeaponMesh());

		// �ѱ��κ��� ũ�ν��� ����Ű�� �ִ� Trace ��ġ����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams);
			}
		}
	}

}
