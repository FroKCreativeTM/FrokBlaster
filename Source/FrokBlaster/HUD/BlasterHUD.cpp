// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	// 뷰포트 사이즈를 가져온다.
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
	// 먼저 텍스처의 X와 Y사이즈를 구한다.
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	
	// 텍스처 위치를 중앙에서 텍스처 사이즈에 맞춰 보정한다
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f),
		ViewportCenter.Y - (TextureHeight / 2.f)
	);

	// 그린다!
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
