#include "BlasterPlayerState.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

// ������ �ٲ�� 
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
			// ���ھ �������� HUD ����
			Controller->SetHUDScore(GetScore());
		}
	}
}

// ���� �󿡼� �й谪 ����
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
	// ���ھ �����Ѵ�.
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
			// ���ھ �������� HUD ����
			Controller->SetHUDScore(GetScore());
		}
	}
}

// �й� ����
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
