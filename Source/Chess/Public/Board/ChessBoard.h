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

	virtual void OnConstruction(const FTransform& Transform);

protected:
	virtual void BeginPlay() override;

#pragma region FUNCTIONS

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "+Chess")
	void InitializeBoard();
	
	void CreateBoard();

	void SetupBoard();

	bool ParseFEN(const FString& FEN, FChessBoardInfo& BoardInfo);

	AChessPiece* SpawnChessPiece(FChessPieceInfo ChessPieceInfo);

	bool HightlightValidMovesOnTile(bool bHighlight, int32 TileIndex);	

	void GenerateAllValidMoves(FChessBoardInfo& BoardInfo, bool bIsWhiteTurn, bool bGenerateOnlyCaptures = false);

	void ClearAllValidMoves(FChessBoardInfo& BoardInfo);

	void UpdateAttackStatusOfTiles(FChessBoardInfo& BoardInfo);

	void UpdateTilesUnderAttackByPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece);

	void CalculateValidMovesForPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> CalculateValidMoveTilesForKing(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> CalculateValidMoveTilesForQueen(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> CalculateValidMoveTilesForBishop(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> CalculateValidMoveTilesForKnight(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> CalculateValidMoveTilesForRook(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> CalculateValidMoveTilesForPawn(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures = false);

	TArray<FChessTileInfo> FilterMovesForCheck(FChessBoardInfo BoardInfo, FChessPieceInfo& Piece, TArray<FChessTileInfo> ValidMovesBeforeFilteration);

	void SimulateMove(FChessBoardInfo& BoardInfo, FChessPieceInfo Piece, int32 ToPosition);

	float MakeMove(AChessTile* StartTile, AChessTile* EndTile, bool bIsAIMove);


	// Check Functions
	bool IsKingInCheck(const FChessBoardInfo& BoardInfo, bool bIsWhiteKing);


	// Enpassant Functions
	void EnableEnpassant(AChessPiece* EnpassantPiece);

	UFUNCTION()
	void DisableEnpassant(bool bIsWhite);


	// AI Functions
	UFUNCTION(BlueprintCallable, Category = "+Chess|Board")
	int32 MoveGenerationTest(FChessBoardInfo BoardInfo, bool bIsWhiteTurn, int32 Depth);
	
	FChessMove CalculateBestAIMove(FChessBoardInfo BoardInfo, bool bIsWhiteTurn, int32 MaxDepth);

	EChessPieceType FindBestPawnPromotionType(FChessBoardInfo BoardInfo, int32 PiecePositionIndex, bool bIsPieceWhite);

	int32 AlphaBetaSearch(FChessBoardInfo BoardInfo, int32 Depth, int32 Alpha, int32 Beta, bool bIsWhiteTurn);

	int32 AlphaBetaSearchAllCaptures(FChessBoardInfo BoardInfo, int32 Depth, int32 Alpha, int32 Beta, bool bIsWhiteTurn);

	int32 EvaluateBoard(const FChessBoardInfo& BoardInfo);

	bool IsEndGame(const FChessBoardInfo& BoardInfo);

	int32 KingPositionScore(int32 KingPosition);

	EChessGameState IsGameOver(const FChessBoardInfo& BoardInfo, bool bIsWhiteTurn);

	bool CanCheckMateOccur(FChessBoardInfo BoardInfo);


	FORCEINLINE bool IsPositionWithinBounds(FVector2D Position)
	{
		return Position.X >= 0 && Position.X < 8 && Position.Y >= 0 && Position.Y < 8;
	}

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

#pragma endregion

#pragma region VARIABLES

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board")
	UChessBoardData* ChessBoardData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board", meta = (AllowedClasses = "/Script/CoreUObject.Class'/Script/Chess.ChessTile'"))
	TSubclassOf<AActor> ChessTileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board", meta = (AllowedClasses = "/Script/CoreUObject.Class'/Script/Chess.ChessPiece'"))
	TSubclassOf<AActor> ChessPieceClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	FString DefaultFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	int32 NumOfMovesSinceLastCapture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Board")
	int32 MaxSearchDepthForAIMove;

	// Enpassant Variables
	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	AChessPiece* EnpassantPawn = nullptr;

	// Opening Variables
	int32 WhiteOpeningMove = 0;
	int32 WhiteOpeningIndex = 0;
	TArray<TArray<FChessMove>> WhiteOpeningMoves = {
		{ FChessMove(12, 28), FChessMove(1, 18) },
		{ FChessMove(11, 27), FChessMove(6, 21) },
		{ FChessMove(6, 23), FChessMove(12, 28) }
	};

	int32 BlackOpeningMove = 0;
	int32 BlackOpeningIndex = 0;
	TArray<TArray<FChessMove>> BlackOpeningMoves = {
		{ FChessMove(52, 36), FChessMove(57, 42) },
		{ FChessMove(51, 35), FChessMove(62, 45) },
		{ FChessMove(52, 44), FChessMove(51, 35) }
	};

private:
	TArray<FVector2D> KingMovePositionTileOffsets = { FVector2D(1, -1), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1), FVector2D(-1, 1), FVector2D(-1, 0), FVector2D(-1, -1), FVector2D(0, -1) };

	TArray<FVector2D> KnightMovePositionTileOffsets = { FVector2D(2, -1), FVector2D(2, 1), FVector2D(1, 2), FVector2D(-1, 2), FVector2D(-2, 1), FVector2D(-2, -1), FVector2D(-1, -2), FVector2D(1, -2) };

	TArray<FVector2D> BishopMovePositionDirections = { FVector2D(1, -1), FVector2D(1, 1), FVector2D(-1, 1), FVector2D(-1, -1) };

	TArray<FVector2D> RookMovePositionDirections = { FVector2D(1, 0), FVector2D(0, 1), FVector2D(-1, 0), FVector2D(0, -1) };

	TArray<FVector2D> QueenMovePositionDirections = KingMovePositionTileOffsets;

#pragma endregion
};