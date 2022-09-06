#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// GameState를 통해서 참가중인 플레이어 리스트를 받아올 수 있다.
	// 여기서는 로비에 있는 플레이어들에 대한 정보를 가져온다.

	// 참가중인 플레이어 수를 가져온다.
	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();

	// 하드코딩 되어있지만 나중에 바꿀 예정
	if (NumOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// 심리스하게
			bUseSeamlessTravel = true;
			// 메인 맵으로 트래블!
			World->ServerTravel("/Game/Maps/BlasterMap.BlasterMap?listen");
		}
	}
}
