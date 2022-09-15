#include "BlasterPlayerController.h"
#include "FrokBlaster/HUD/BlasterHUD.h"
#include "FrokBlaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// �÷��̾� ��Ʈ�ѷ��� HUD�� ������ �� �ִ�.
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	// ���� nullptr üũ�� �� �� BeginPlay���� ����� �Ҵ��� ���� �ʾҴٸ�
	// �ٽ� GetHUD�� �̿��ؼ� �����´�.
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	// ���� HUD�� ��� ������ �� �Ҵ��� �Ǿ��°��� üũ�Ѵ�.
	bool bHUDValid = (BlasterHUD != nullptr &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText);

	if (bHUDValid)
	{
		// ���� ü���� �ۼ�Ƽ���� ����Ѵ�.
		const float HealthPercent = Health / MaxHealth;

		// HUD�ٿ� �̸� �ݿ��Ѵ�.
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		// ü�� �ؽ�Ʈ�� �����Ѵ�.
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}


}


