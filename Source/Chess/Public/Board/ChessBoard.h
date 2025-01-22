// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Miscellaneous/StructuresAndEnumerations.h"

#include "GameFramework/Actor.h"

#include "ChessBoard.generated.h"

class AChessPiece;
class AChessTile;
class UChessBoardData;

UCLASS()
class CHESS_API AChessBoard : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRootComponent = nullptr;

public:
	AChessBoard();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#pragma region FUNCTIONS

public:
	void CreateBoard();

	void SetupBoard();

	AChessPiece* SpawnChessPiece(FChessPieceInfo ChessPieceInfo);

	bool HightlightValidMovesOnTile(bool bHighlight, int32 TileIndex);	

	void GenerateAllValidMoves(FChessBoardInfo& BoardInfo, bool bIsWhiteTurn);

	void ClearAllValidMoves(FChessBoardInfo& BoardInfo);

	void UpdateAttackStatusOfTiles(FChessBoardInfo& BoardInfo);

	void UpdateTilesUnderAttackByPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	void CalculateValidMovesForPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> CalculateValidMoveTilesForKing(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> CalculateValidMoveTilesForQueen(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> CalculateValidMoveTilesForBishop(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> CalculateValidMoveTilesForKnight(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> CalculateValidMoveTilesForRook(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> CalculateValidMoveTilesForPawn(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	TArray<FChessTileInfo> FilterMovesForCheck(FChessBoardInfo BoardInfo, FChessPieceInfo& Piece, TArray<FChessTileInfo> ValidMovesBeforeFilteration);

	void SimulateMove(FChessBoardInfo& BoardInfo, FChessPieceInfo Piece, int32 ToPosition);

	void MakeMove(AChessTile* StartTile, AChessTile* EndTile);


	// Check Functions
	bool IsKingInCheck(const FChessBoardInfo& BoardInfo, bool bIsWhiteKing);


	// Enpassant Functions
	void EnableEnpassant(AChessPiece* EnpassantPiece);

	UFUNCTION()
	void DisableEnpassant(bool bIsWhite);


	FORCEINLINE bool IsPositionWithinBounds(FVector2D Position) { return Position.X >= 0 && Position.X < 8 && Position.Y >= 0 && Position.Y < 8; }

	FORCEINLINE AChessTile* GetChessTileAtPosition(FVector2D Position) const
	{
		if (Position.X < 0 || Position.Y < 0 || Position.X > 7 || Position.Y > 7) return nullptr;
		if (!ChessTiles.IsValidIndex((Position.X * 8 + Position.Y))) return nullptr;
		return ChessTiles[(Position.X * 8 + Position.Y)];
	}

	FORCEINLINE AChessTile* GetChessPieceAtPosition(FVector2D Position) const
	{
		if (Position.X < 0 || Position.Y < 0 || Position.X > 7 || Position.Y > 7) return nullptr;
		if (!ChessTiles.IsValidIndex((Position.X * 8 + Position.Y))) return nullptr;
		return ChessTiles[(Position.X * 8 + Position.Y)];
	}


	UFUNCTION(BlueprintCallable, Category = "+Chess|Board")
	int32 MoveGenerationTest(FChessBoardInfo BoardInfo, bool bIsWhiteTurn, int32 Depth);

#pragma endregion

#pragma region VARIABLES

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board")
	UChessBoardData* ChessBoardData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board", meta = (AllowedClasses = "/Script/CoreUObject.Class'/Script/Chess.ChessTile'"))
	TSubclassOf<AActor> ChessTileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board", meta = (AllowedClasses = "/Script/CoreUObject.Class'/Script/Chess.ChessPiece'"))
	TSubclassOf<AActor> ChessPieceClass = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<AChessPiece*> WhiteChessPieces;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<AChessPiece*> BlackChessPieces;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FChessTileInfo> HighlightedTiles;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FVector> ChessTileLocations;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<AChessTile*> ChessTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	FChessBoardInfo ChessBoardInfo;

	// Enpassant Variables
	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	AChessPiece* EnpassantPawn = nullptr;

private:
	TArray<FVector2D> KingMovePositionTileOffsets = { FVector2D(1, -1), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1), FVector2D(-1, 1), FVector2D(-1, 0), FVector2D(-1, -1), FVector2D(0, -1) };

	TArray<FVector2D> KnightMovePositionTileOffsets = { FVector2D(2, -1), FVector2D(2, 1), FVector2D(1, 2), FVector2D(-1, 2), FVector2D(-2, 1), FVector2D(-2, -1), FVector2D(-1, -2), FVector2D(1, -2) };

	TArray<FVector2D> BishopMovePositionDirections = { FVector2D(1, -1), FVector2D(1, 1), FVector2D(-1, 1), FVector2D(-1, -1) };

	TArray<FVector2D> RookMovePositionDirections = { FVector2D(1, 0), FVector2D(0, 1), FVector2D(-1, 0), FVector2D(0, -1) };

	TArray<FVector2D> QueenMovePositionDirections = KingMovePositionTileOffsets;

#pragma endregion
};