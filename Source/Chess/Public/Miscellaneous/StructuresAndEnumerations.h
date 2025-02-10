// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "StructuresAndEnumerations.generated.h"


// Enumerations

UENUM(BlueprintType)
enum class EChessPieceType : uint8
{
	King			UMETA(DisplayName = "King"),
	Queen			UMETA(DisplayName = "Queen"),
	Bishop			UMETA(DisplayName = "Bishop"),
	Knight			UMETA(DisplayName = "Knight"),
	Rook			UMETA(DisplayName = "Rook"),
	Pawn			UMETA(DisplayName = "Pawn")
};

UENUM(BlueprintType)
enum class EChessPieceSide : uint8
{
	None			UMETA(DisplayName = "None"),
	KingSide		UMETA(DisplayName = "KingSide"),
	QueenSide		UMETA(DisplayName = "QueenSide")
};

UENUM(BlueprintType)
enum class EChessGameState : uint8
{
	Ongoing			UMETA(DisplayName = "Ongoing"),
	Draw			UMETA(DisplayName = "Draw"),
	Stalemate		UMETA(DisplayName = "Stalemate"),
	Checkmate		UMETA(DisplayName = "Checkmate")
};

UENUM(BlueprintType)
enum class EChessGameModeType : uint8
{
	Player_VS_AI			UMETA(DisplayName = "Player VS AI"),
	Player_VS_Player		UMETA(DisplayName = "Player VS Player")
};

UENUM(BlueprintType)
enum class EChessMapType : uint8
{
	Beach				UMETA(DisplayName = "Beach"),
	SnowyForest			UMETA(DisplayName = "SnowyForest"),
	MapleForest			UMETA(DisplayName = "MapleForest"),
	ConiferForest		UMETA(DisplayName = "ConiferForest")
};



// Structures

USTRUCT(BlueprintType)
struct FChessMapData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChessMapType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Map;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;
	
	FChessMapData() {}
};

USTRUCT(BlueprintType)
struct FChessMove
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FromIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ToIndex;

	FChessMove() : FromIndex(-1), ToIndex(-1) {}

	FChessMove(int32 FromIndex_, int32 ToIndex_) : 
		FromIndex(FromIndex_),
		ToIndex(ToIndex_) {
	}
};

USTRUCT(BlueprintType)
struct FChessPieceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "IsWhite?"))
	bool bIsWhite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChessPieceType ChessPieceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChessPieceSide ChessPieceSide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChessPiecePositionIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> ValidMoves;

	FChessPieceInfo() :
		bIsWhite(true),
		ChessPieceType(EChessPieceType::Pawn),
		ChessPieceSide(EChessPieceSide::None),
		ChessPiecePositionIndex(-1) {
	}

	FChessPieceInfo(bool bIsWhite_, EChessPieceType ChessPieceType_, EChessPieceSide ChessPieceSide_, int32 ChessPiecePositionIndex_, TArray<int32> ValidMoves_) :
		bIsWhite(bIsWhite_),
		ChessPieceType(ChessPieceType_),
		ChessPieceSide(ChessPieceSide_),
		ChessPiecePositionIndex(ChessPiecePositionIndex_),
		ValidMoves(ValidMoves_) {
	}

	bool operator==(const FChessPieceInfo& Other) const
	{
		return bIsWhite == Other.bIsWhite && ChessPieceType == Other.ChessPieceType && ChessPieceSide == Other.ChessPieceSide && ChessPiecePositionIndex == Other.ChessPiecePositionIndex && ValidMoves == Other.ValidMoves;
	}

	FVector2D GetChessPiecePositionFromIndex()
	{
		return FVector2D(ChessPiecePositionIndex / 8, ChessPiecePositionIndex % 8);
	}

	//int32 GetChessPiecePositionAsIndex()
	//{
	//	return ChessPiecePosition.X * 8 + ChessPiecePosition.Y;
	//}
};

USTRUCT(BlueprintType)
struct FChessTileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FChessPieceInfo ChessPieceOnTile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChessTilePositionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWhite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHighlighted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTileUnderAttackByWhitePiece;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTileUnderAttackByBlackPiece;

	FChessTileInfo() :
		ChessPieceOnTile(FChessPieceInfo()),
		ChessTilePositionIndex(-1),
		bIsWhite(true),
		bIsHighlighted(false),
		bIsTileUnderAttackByWhitePiece(false),
		bIsTileUnderAttackByBlackPiece(false) {
	}

	bool operator==(const FChessTileInfo& Other) const
	{
		return ChessPieceOnTile == Other.ChessPieceOnTile && ChessTilePositionIndex == Other.ChessTilePositionIndex && bIsWhite == Other.bIsWhite && bIsHighlighted == Other.bIsHighlighted && bIsTileUnderAttackByWhitePiece == Other.bIsTileUnderAttackByWhitePiece && bIsTileUnderAttackByBlackPiece == Other.bIsTileUnderAttackByBlackPiece;
	}

	FVector2D GetChessTilePositionFromIndex()
	{
		return FVector2D(ChessTilePositionIndex / 8, ChessTilePositionIndex % 8);
	}
};

USTRUCT(BlueprintType)
struct FChessBoardInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FChessTileInfo> TilesInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWhiteTurn;

	// Check variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWhiteKingUnderCheck;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBlackKingUnderCheck;


	// EnPassant Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FChessPieceInfo EnpassantPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 EnpassantTileIndex;


	// Castling variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWhiteKingSideRookAlive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBlackKingSideRookAlive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWhiteQueenSideRookAlive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBlackQueenSideRookAlive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasWhiteKingMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBlackKingMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasWhiteKingSideRookMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBlackKingSideRookMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasWhiteQueenSideRookMoved;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBlackQueenSideRookMoved;


	FChessBoardInfo() :
		bIsWhiteTurn(true),
		bIsWhiteKingUnderCheck(false),
		bIsBlackKingUnderCheck(false),
		EnpassantPawn(FChessPieceInfo()),
		EnpassantTileIndex(-1),
		bIsWhiteKingSideRookAlive(true),
		bIsBlackKingSideRookAlive(true),
		bIsWhiteQueenSideRookAlive(true),
		bIsBlackQueenSideRookAlive(true),
		bHasWhiteKingMoved(false),
		bHasBlackKingMoved(false),
		bHasWhiteKingSideRookMoved(false),
		bHasBlackKingSideRookMoved(false),
		bHasWhiteQueenSideRookMoved(false),
		bHasBlackQueenSideRookMoved(false) {
	}


	// Castling Functions
	FORCEINLINE bool HasWhiteKingOrKingSideRookMoved()	const { return (bHasWhiteKingMoved || bHasWhiteKingSideRookMoved); }
	FORCEINLINE bool HasWhiteKingOrQueenSideRookMoved() const { return (bHasWhiteKingMoved || bHasWhiteQueenSideRookMoved); }
	FORCEINLINE bool HasBlackKingOrKingSideRookMoved()	const { return (bHasBlackKingMoved || bHasBlackKingSideRookMoved); }
	FORCEINLINE bool HasBlackKingOrQueenSideRookMoved() const { return (bHasBlackKingMoved || bHasBlackQueenSideRookMoved); }
};