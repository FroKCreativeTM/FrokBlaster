#include "BlasterGameMode.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, 
	ABlasterPlayerController* VictimController, 
	ABlasterPlayerController* AttackerController)
{
	// 캐릭터가 죽었을 경우 
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
