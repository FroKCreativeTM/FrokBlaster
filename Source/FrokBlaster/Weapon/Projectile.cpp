#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "FrokBlaster/FrokBlaster.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;	// ��� Ŭ���̾�Ʈ�� �����Ѵ�.

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	// �� �ݸ����� ���� �󿡼� �̵��ϸ�
	// ������ �������� ������ �޽��ϴ�.
	// ��� ä�ο� ���ؼ��� ����ϸ� ����������
	// ���� ���̴� ��ü��, ���忡 ������ ��(��)�鿡 ���ؼ��� ������� ���մϴ�.
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,
		ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh,
		ECollisionResponse::ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// ����ü�� ȸ���� �̵� ���⿡ ���� �� ������ ������Ʈ
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// ����ü�� �ʱ� ���ǵ�� �ְ� ���ǵ�� 15000�̴�.
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

	// ������ ���
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	// ���� ���������� ������ �� Ŭ������ ��ӹ޴�
	// ���� ����ü Ŭ�������� �����Ѵ�.

	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		// ���� ���� ��ƼŬ ȿ���� �����Ѵ�.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			ImpactParticles,
			GetActorTransform());
	}

	if (ImpactSound)
	{
		// ���� ������ ���带 ����Ѵ�.
		UGameplayStatics::PlaySoundAtLocation(this,
			ImpactSound,
			GetActorLocation());
	}
}

