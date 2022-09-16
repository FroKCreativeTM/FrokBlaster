#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * HUD 등의 관리를 하기 위한 플레이어 컨트롤러
 * 서버-클라이언트 입장에서는 알아둬야 할 것이
 * 멀티 플레이어 상에서는 플레이어 컨트롤러는
 * 기본적으로 권한을 가진(Owning) 플레이어에게 플레이어 컨트롤러 하나가 할당된다.
 * 이것을 이용해서 HUD 등의 데이터에 대한 정보를 관리할 수 있게 된다.
 */
UCLASS()
class FROKBLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public : 
	void SetHUDHealth(float Health, float MaxHealth);

	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);

protected : 
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private : 
	class ABlasterHUD* BlasterHUD;
};
