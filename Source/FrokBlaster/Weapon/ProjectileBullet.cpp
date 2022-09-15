#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	// 이 머신이 권한을 가지고 있는 Character를 가져온다.
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		// 권한 하의 캐릭터의 컨트롤러를 가져온다.
		AController* OwnerController = OwnerCharacter->Controller;

		if (OwnerController)
		{
			// 데미지를 가한다.
			UGameplayStatics::ApplyDamage(OtherActor, 
				Damage, OwnerController, 
				this, UDamageType::StaticClass());
		}
	}

	Super::OnHit(HitComp,
		OtherActor,
		OtherComp,
		NormalImpulse,
		Hit);
}
