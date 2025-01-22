// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/ChessGameInstance.h"

#include "GameFramework/GameMode.h"

#include "ChessGameMode.generated.h"

class AChessBoard;
class AChessPlayer;
class AChessPlayerController;
class AChessTile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwitchTurn);

UCLASS()
class CHESS_API AChessGameMode : public AGameMode
{
	GENERATED_BODY()

public:
    AChessGameMode();

    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    virtual void BeginPlay() override;

#pragma region FUNCTION

public:
    void SwitchTurn();

#pragma endregion

#pragma region VARIABLES

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|GameMode", meta = (AllowedClasses = "/Script/CoreUObject.Class'/Script/Chess.ChessBoard'"))
    TSubclassOf<AActor> ChessBoardClass = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameMode")
    AChessBoard* ChessBoard = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameMode")
    AChessPlayerController* ChessPlayerController = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameMode")
    AChessPlayer* ChessPlayer = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameMode")
    EChessGameModeType ChessGameModeType;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|GameMode")
    bool bIsWhiteTurn;

    UPROPERTY(BlueprintAssignable, Category = "+Chess|GameMode")
    FOnSwitchTurn OnSwitchTurn;

#pragma endregion
};