#include "BlasterGameMode.h"
#include "FrokBlaster/Character/BlasterCharacter.h"
#include "FrokBlaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "FrokBlaster/PlayerState/BlasterPlayerState.h"

// Ư�� �÷��̾ ���� ���
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, 
	ABlasterPlayerController* VictimController, 
	ABlasterPlayerController* AttackerController)
{
	// ���� �� �Ǵ� ��Ȳ���� ���� üũ�� �Ѵ�.
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	// ������ �÷��̾� ���¸� �����´�. (�� ���� ���ؽ�Ʈ�� �����´�.)
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	// �̱� ������� ����, �� ������� �й踦 �ش�.
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	// �й��� ����� �����Ѵ�.
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter,
	AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
