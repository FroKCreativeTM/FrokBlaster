#include "BlasterGameMode.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, 
	ABlasterPlayerController* VictimController, 
	ABlasterPlayerController* AttackerController)
{
	// ĳ���Ͱ� �׾��� ��� 
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
