#include "GameState.h"
#include "MainScene.h"
#include "WorldNode.h"

GameState::GameState(MainScene& scene)
	: mScene{scene}
{
	mGamePhase = GamePhase::ReadyToRollDice;
}

void GameState::Next()
{
	if (mGamePhase == GamePhase::ReadyToRollDice)
	{
		mGamePhase = GamePhase::DiceRolling;
	}
	else if (mGamePhase == GamePhase::DiceRolling)
	{
		mGamePhase = GamePhase::DiceStop;
	}
	else if (mGamePhase == GamePhase::DiceStop)
	{
		mGamePhase = GamePhase::PickPlace;
	}
	else if (mGamePhase == GamePhase::PickPlace)
	{
		mGamePhase = GamePhase::CharacterMoving;
	}
	else if (mGamePhase == GamePhase::CharacterMoving)
	{
		mGamePhase = GamePhase::ReadyToRollDice;
	}
}

GamePhase GameState::GetPhase()
{
	return mGamePhase;
}

void GameState::Update()
{
	if (mGamePhase == GamePhase::ReadyToRollDice)
	{
		mScene.mCameraNode->SetAngle(60);
		mScene.mCameraNode->LookDown();
		mScene.mCameraNode->SetRelativePosition(100.0f, 50.0f, 0.0f);
		mScene.GetDiceSystem()->mInputComponent->Activate();
		mScene.GetEarth() ->mInputComponent->Deactivate();
	}
	else if (mGamePhase == GamePhase::DiceRolling)
	{
		mScene.GetDiceSystem()->mInputComponent->Deactivate();
		if (mScene.mPhysicsManager->GetSystemVelocity() < 1.0f)
		{
			Next();
		}
	}
	else if (mGamePhase == GamePhase::DiceStop)
	{
		printf("sum : %d\n", mScene.GetDiceSystem()->UpperSide());
		Next();
	}
	else if (mGamePhase == GamePhase::PickPlace)
	{
		mScene.mCameraNode->LookFront();
		mScene.mCameraNode->SetRelativePosition(0.0f, 0.0f, -35.0f);
		mScene.GetEarth()->mInputComponent->Activate();
	}
	else if (mGamePhase == GamePhase::CharacterMoving)
	{
		mScene.GetEarth()->mInputComponent->Deactivate();
		if (!mScene.GetEarth()->GetIsMoving())
			Next();
	}
}