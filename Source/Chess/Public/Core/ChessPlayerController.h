// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"

#include "ChessPlayerController.generated.h"

class AChessTile;
class AChessPiece;

class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPieceMoved, bool, bIsWhite);

UCLASS()
class CHESS_API AChessPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    AChessPlayerController();

protected:
    virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

#pragma region FUNCTIONS

public:
    void SelectPiece();

    UFUNCTION(BlueprintImplementableEvent, Category = "+Chess|PlayerController")
    void SpawnPawnPromotionUI(AChessPiece* PawnPiece);

#pragma endregion

#pragma region VARIABLES

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|PlayerController|Input")
    UInputMappingContext* InputMappingContext = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|PlayerController|Input")
    UInputAction* SelectPieceAction = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|PlayerController")
    AChessTile* SelectedTile = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "+Chess|PlayerController")
    bool bIsPlayerTurn = true;

    UPROPERTY(BlueprintAssignable, Category = "+Chess|PlayerController")
    FOnPieceMoved OnPieceMoved;

#pragma endregion
};