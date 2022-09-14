// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	// ����Ʈ ����� �����´�.
	FVector2D ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

		if (HUDPackage.CrosshairsCenter)
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter);
		if (HUDPackage.CrosshairsLeft)
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter);
		if (HUDPackage.CrosshairsRight)
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter);
		if (HUDPackage.CrosshairsTop)
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter);
		if (HUDPackage.CrosshairsBottom)
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter);
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	// ���� �ؽ�ó�� X�� Y����� ���Ѵ�.
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	
	// �ؽ�ó ��ġ�� �߾ӿ��� �ؽ�ó ����� ���� �����Ѵ�
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f),
		ViewportCenter.Y - (TextureHeight / 2.f)
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
		FLinearColor::White);

}