// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FROKBLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	// 플레이어 제거 기능
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, 
		class ABlasterPlayerController* VictimController, 
		ABlasterPlayerController* AttackerController);
};
