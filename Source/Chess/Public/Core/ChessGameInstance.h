// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ChessGameInstance.generated.h"

UENUM(BlueprintType)
enum class EChessGameModeType : uint8
{
	Player_VS_AI			UMETA(DisplayName = "Player VS AI"),
	Player_VS_Player		UMETA(DisplayName = "Player VS Player")
};

UCLASS()
class CHESS_API UChessGameInstance : public UGameInstance
{
	GENERATED_BODY()

#pragma region FUNCTIONS

public:
	UFUNCTION(BlueprintCallable, Category = "+Chess|GameInstance")
	void UpdateChessGameModeType(EChessGameModeType NewChessGameModeType);

#pragma endregion

#pragma region VARIABLES

public:
	UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameInstance")
	EChessGameModeType ChessGameModeType;

#pragma endregion
};