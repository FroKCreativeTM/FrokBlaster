#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// 총구 부분의 Socket을 가져온다.
	const USkeletalMeshSocket* MuzzleSocket 
		= GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	// 총구가 존재한다.
	if (MuzzleSocket)
	{
		// 총구의 트랜스폼 정보를 가져온다.
		FTransform SocketTransform 
			= MuzzleSocket->GetSocketTransform(GetWeaponMesh());

		// 총구로부터 크로스헤어가 가리키고 있는 Trace 위치까지
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
