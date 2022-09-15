#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * HUD ���� ������ �ϱ� ���� �÷��̾� ��Ʈ�ѷ�
 * ����-Ŭ���̾�Ʈ ���忡���� �˾Ƶ־� �� ����
 * ��Ƽ �÷��̾� �󿡼��� �÷��̾� ��Ʈ�ѷ���
 * �⺻������ ������ ����(Owning) �÷��̾�� �÷��̾� ��Ʈ�ѷ� �ϳ��� �Ҵ�ȴ�.
 * �̰��� �̿��ؼ� HUD ���� �����Ϳ� ���� ������ ������ �� �ְ� �ȴ�.
 */
UCLASS()
class FROKBLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public : 
	void SetHUDHealth(float Health, float MaxHealth);

protected : 
	virtual void BeginPlay() override;

private : 
	class ABlasterHUD* BlasterHUD;


};
