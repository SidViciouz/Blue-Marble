#pragma once

class MainScene;

enum class GamePhase
{
	ReadyToRollDice,
	DiceRolling,
	DiceStop,
	PickPlace,
	CharacterMoving
};

class GameState
{
public:
												GameState(MainScene& scene);
												GameState(const GameState& gameState) = delete;
												GameState(GameState&& gameState) = delete;
	GameState&									operator=(const GameState& gameState) = delete;
	GameState&									operator=(GameState&& gameState) = delete;

	void										Next();
	GamePhase									GetPhase();
	void										Update();

protected:
	GamePhase									mGamePhase;
	MainScene&									mScene;
};
