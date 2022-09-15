#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	// ����Ʈ ����� �����´�.
	FVector2D ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	// ���� ���� Ŭ���̾�Ʈ���� ���� ��Ʈ�ѷ��� �����´�.
	APlayerController * PlayerController = GetOwningPlayerController();	

	if (PlayerController && CharacterOverlayClass)
	{
		// ��Ʈ�ѷ��� �̿��ؼ� CharacterOverlayŬ���� ��ü�� �����Ѵ�.
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter,
	FVector2D Spread, FLinearColor CrosshairColor)
{
	// ���� �ؽ�ó�� X�� Y����� ���Ѵ�.
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	
	// �ؽ�ó ��ġ�� �߾ӿ��� �ؽ�ó ����� ���� �����Ѵ�.
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	// �׸���!
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f, 0.f,
		1.f, 1.f,
		CrosshairColor);

}
