// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Miscellaneous/StructuresAndEnumerations.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "ChessPiece.generated.h"

class AChessBoard;
class AChessTile;
class UChessBoardData;

class UInterpToMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPieceCaptured);

UCLASS()
class CHESS_API AChessPiece : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRootComponent = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ChessPieceMesh = nullptr;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece", meta = (AllowPrivateAccess = "true"))
	UInterpToMovementComponent* InterpToMovementComponent = nullptr;

public:
	AChessPiece();

	virtual void OnConstruction(const FTransform& Transform) override;

	FORCEINLINE UStaticMeshComponent* GetChessPieceMesh() const { return ChessPieceMesh; }
	FORCEINLINE UInterpToMovementComponent* GetInterpToMovementComponent() const { return InterpToMovementComponent; }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

#pragma region FUNCTIONS

public:
	void UpdateChessPieceStaticMesh();

	void CapturePiece();

	void MovePiece(AChessTile* MoveToTile);

	//void UpdateTilesUnderAttack();

	//FORCEINLINE bool IsPositionWithinBounds(FVector2D Position) { return Position.X >= 0 && Position.X < 8 && Position.Y >= 0 && Position.Y < 8; }

	UFUNCTION(BlueprintCallable, Category = "+Chess|Piece")
	void PromotePawn(EChessPieceType PromotionType);

#pragma endregion

#pragma region VARIABLES

public:
	float TotalTravelDistance = 0.f;

	FOnPieceCaptured OnPieceCaptured;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece")
	UChessBoardData* ChessBoardData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Piece")
	AChessBoard* ChessBoard = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Piece")
	FChessPieceInfo ChessPieceInfo;

private:
	FPredictProjectilePathParams PredictParams;

	//TArray<FVector2D> KingMovePositionTileOffsets = { FVector2D(1, -1), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1), FVector2D(-1, 1), FVector2D(-1, 0), FVector2D(-1, -1), FVector2D(0, -1) };

	//TArray<FVector2D> KnightMovePositionTileOffsets = { FVector2D(2, -1), FVector2D(2, 1), FVector2D(1, 2), FVector2D(-1, 2), FVector2D(-2, 1), FVector2D(-2, -1), FVector2D(-1, -2), FVector2D(1, -2) };

	//TArray<FVector2D> BishopMovePositionDirections = { FVector2D(1, -1), FVector2D(1, 1), FVector2D(-1, 1), FVector2D(-1, -1) };

	//TArray<FVector2D> RookMovePositionDirections = { FVector2D(1, 0), FVector2D(0, 1), FVector2D(-1, 0), FVector2D(0, -1) };

	//TArray<FVector2D> QueenMovePositionDirections = KingMovePositionTileOffsets;

#pragma endregion
};