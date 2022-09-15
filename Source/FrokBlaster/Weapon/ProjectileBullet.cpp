#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	// �� �ӽ��� ������ ������ �ִ� Character�� �����´�.
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		// ���� ���� ĳ������ ��Ʈ�ѷ��� �����´�.
		AController* OwnerController = OwnerCharacter->Controller;

		if (OwnerController)
		{
			// �������� ���Ѵ�.
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
