#include "BlasterPlayerState.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

// 점수가 바뀌면 
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->GetBlasterCharacterController())
	{
		Controller
			= Controller == nullptr ?
			Character->GetBlasterCharacterController() :
			Controller;

		if (Controller)
		{
			// 스코어를 바탕으로 HUD 갱신
			Controller->SetHUDScore(GetScore());
		}
	}
}

// 서버 상에서 패배값 갱신
void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->GetBlasterCharacterController())
	{
		Controller
			= Controller == nullptr ?
			Character->GetBlasterCharacterController() :
			Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	// 스코어를 갱신한다.
	SetScore(GetScore() + ScoreAmount);	// super function 

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->GetBlasterCharacterController())
	{
		Controller
			= Controller == nullptr ?
			Character->GetBlasterCharacterController() :
			Controller;

		if (Controller)
		{
			// 스코어를 바탕으로 HUD 갱신
			Controller->SetHUDScore(GetScore());
		}
	}
}

// 패배 갱신
void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->GetBlasterCharacterController())
	{
		Controller
			= Controller == nullptr ?
			Character->GetBlasterCharacterController() :
			Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
