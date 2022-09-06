#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// GameState�� ���ؼ� �������� �÷��̾� ����Ʈ�� �޾ƿ� �� �ִ�.
	// ���⼭�� �κ� �ִ� �÷��̾�鿡 ���� ������ �����´�.

	// �������� �÷��̾� ���� �����´�.
	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();

	// �ϵ��ڵ� �Ǿ������� ���߿� �ٲ� ����
	if (NumOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// �ɸ����ϰ�
			bUseSeamlessTravel = true;
			// ���� ������ Ʈ����!
			World->ServerTravel("/Game/Maps/BlasterMap.BlasterMap?listen");
		}
	}
}
