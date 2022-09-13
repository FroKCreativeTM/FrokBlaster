#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;	// 모든 클라이언트에 복제한다.

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	// 이 콜리전은 월드 상에서 이동하며
	// 쿼리와 물리학의 영향을 받습니다.
	// 모든 채널에 대해서는 통과하며 지나가지만
	// 만약 보이는 물체나, 월드에 고정된 것(벽)들에 대해서는 통과하지 못합니다.
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,
		ECollisionResponse::ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// 투사체의 회전이 이동 방향에 맞춰 매 프레임 업데이트
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// 투사체의 초기 스피드와 최고 스피드는 15000이다.
	ProjectileMovementComponent->InitialSpeed = 15000.f;
	ProjectileMovementComponent->MaxSpeed = 15000.f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition);
	}

	// 서버의 경우
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	// 사용한 투사체를 삭제한다. (아래 Destroyed를 실행한다.)
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		// 맞은 곳에 파티클 효과를 실행한다.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			ImpactParticles,
			GetActorTransform());
	}

	if (ImpactSound)
	{
		// 맞은 곳에서 사운드를 재생한다.
		UGameplayStatics::PlaySoundAtLocation(this,
			ImpactSound,
			GetActorLocation());
	}
}

