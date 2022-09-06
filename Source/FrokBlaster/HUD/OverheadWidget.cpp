#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	}

	FString LocalRoleString = FString::Printf(TEXT("Local Role : %s"), *Role);
	SetDisplayText(LocalRoleString);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	// UI를 지우고 월드로부터 사라진다.
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
