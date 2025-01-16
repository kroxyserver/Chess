// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "ChessBoard.generated.h"

class AChessPiece;
class AChessTile;
class UChessBoardData;
struct FChessPieceInfo;
struct FChessTileInfo;

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

	void UpdateAttackStatusOfTiles();

	void ClearAllValidMoves();

	UFUNCTION()
	void GenerateAllValidMoves(bool bIsWhiteTurn);

	bool HightlightValidMovesOnTile(bool bHighlight, FChessTileInfo ChessTileInfo);
	
	FORCEINLINE AChessTile* GetChessTileAtPosition(FVector2D Position) const
	{
		if (Position.X < 0 || Position.Y < 0 || Position.X > 7 || Position.Y > 7) return nullptr;
		if (!ChessTiles.IsValidIndex((Position.X * 8 + Position.Y))) return nullptr;
		return ChessTiles[(Position.X * 8 + Position.Y)];
	}



	// Enpassant Functions
	void EnableEnpassant(AChessPiece* EnpassantPiece);

	UFUNCTION()
	void DisableEnpassant(bool bIsWhite);



	// Check Functions
	bool IsKingInCheck(bool bIsWhiteKing, const TArray<FChessTileInfo>& BoardLayout, int32 OutEnpassantTarget) const;



	// Castling Functions
	FORCEINLINE bool HasWhiteKingOrKingSideRookMoved()	const { return (bHasWhiteKingMoved || bHasWhiteKingSideRookMoved); }
	FORCEINLINE bool HasWhiteKingOrQueenSideRookMoved() const { return (bHasWhiteKingMoved || bHasWhiteQueenSideRookMoved); }
	FORCEINLINE bool HasBlackKingOrKingSideRookMoved() const { return (bHasBlackKingMoved || bHasBlackKingSideRookMoved); }
	FORCEINLINE bool HasBlackKingOrQueenSideRookMoved() const { return (bHasBlackKingMoved || bHasBlackQueenSideRookMoved); }

#pragma endregion

#pragma region VARIABLES

public:
	TArray<uint8> TileColourAtIndex = { 0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0 };

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FChessTileInfo> ChessBoardLayout;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FChessTileInfo> HighlightedTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FChessTileInfo> ChessTilesUnderAttackByWhitePieces;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FChessTileInfo> ChessTilesUnderAttackByBlackPieces;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<FVector> ChessTileLocations;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	TArray<AChessTile*> ChessTiles;


	// Check variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	bool bIsWhiteKingUnderCheck = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	bool bIsBlackKingUnderCheck = false;



	// EnPassant Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	int32 EnpassantTileIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Board")
	AChessPiece* EnpassantPawn = nullptr;

	
	
	// Castling variables
	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bIsWhiteKingSideRookAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bIsBlackKingSideRookAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bIsWhiteQueenSideRookAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bIsBlackQueenSideRookAlive = true;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bHasWhiteKingMoved = false;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bHasBlackKingMoved = false;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bHasWhiteKingSideRookMoved = false;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bHasBlackKingSideRookMoved = false;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bHasWhiteQueenSideRookMoved = false;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Board")
	bool bHasBlackQueenSideRookMoved = false;

#pragma endregion
};