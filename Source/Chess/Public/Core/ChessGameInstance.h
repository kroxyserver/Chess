// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Miscellaneous/StructuresAndEnumerations.h"

#include "Engine/GameInstance.h"

#include "ChessGameInstance.generated.h"

UCLASS()
class CHESS_API UChessGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UChessGameInstance();

#pragma region FUNCTIONS

public:
	UFUNCTION(BlueprintCallable, Category = "+Chess|GameInstance")
	void UpdateChessGameModeType(EChessGameModeType NewChessGameModeType);

	UFUNCTION(BlueprintCallable, Category = "+Chess|GameInstance")
	void UpdateChessMapType(EChessMapType NewChessMapType);

#pragma endregion

#pragma region VARIABLES

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|GameInstance")
	UDataTable* ChessMapDataTable = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameInstance")
	EChessGameModeType ChessGameModeType;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameInstance")
	EChessMapType ChessMapType;

#pragma endregion
};