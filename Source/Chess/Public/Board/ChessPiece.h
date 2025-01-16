// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "ChessPiece.generated.h"

class AChessBoard;
class AChessTile;
class UChessBoardData;
struct FChessTileInfo;

class UInterpToMovementComponent;

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

USTRUCT(BlueprintType)
struct FChessPieceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "IsWhite?"))
	bool bIsWhite;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "HasMoved?"))
	bool bHasMoved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChessPieceType ChessPieceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChessPieceSide ChessPieceSide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChessPiecePositionIndex;

	FChessPieceInfo() :
		bIsWhite(true),
		bHasMoved(false),
		ChessPieceType(EChessPieceType::Pawn),
		ChessPieceSide(EChessPieceSide::None),
		ChessPiecePositionIndex(0) {}

	FChessPieceInfo(bool bIsWhite_, bool bHasMoved_, EChessPieceType ChessPieceType_, EChessPieceSide ChessPieceSide_, int32 ChessPiecePositionIndex_) :
		bIsWhite(bIsWhite_),
		bHasMoved(bHasMoved_),
		ChessPieceType(ChessPieceType_),
		ChessPieceSide(ChessPieceSide_),
		ChessPiecePositionIndex(ChessPiecePositionIndex_) {}

	bool operator==(const FChessPieceInfo& Other) const
	{
		return bIsWhite == Other.bIsWhite && bHasMoved == Other.bHasMoved && ChessPieceType == Other.ChessPieceType && ChessPieceSide == Other.ChessPieceSide && ChessPiecePositionIndex == Other.ChessPiecePositionIndex;
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

	TArray<FChessTileInfo> CalculateValidMoveTilesForKing(TArray<FChessTileInfo> Board);

	TArray<FChessTileInfo> CalculateValidMoveTilesForQueen(TArray<FChessTileInfo> Board);

	TArray<FChessTileInfo> CalculateValidMoveTilesForBishop(TArray<FChessTileInfo> Board);

	TArray<FChessTileInfo> CalculateValidMoveTilesForKnight(TArray<FChessTileInfo> Board);

	TArray<FChessTileInfo> CalculateValidMoveTilesForRook(TArray<FChessTileInfo> Board);

	TArray<FChessTileInfo> CalculateValidMoveTilesForPawn(TArray<FChessTileInfo> Board, int32 OutEnpassantTarget);

	void CapturePiece();

	void CalculateValidMoves();

	TArray<FChessTileInfo> FilterMovesForCheck(TArray<FChessTileInfo>& ValidMovesBeforeFilteration);

	void SimulateMove(int32 ToPosition, TArray<FChessTileInfo>& BoardLayout, int32& OutEnPassantTarget, bool bIsWhiteTurn);

	void MovePiece(AChessTile* MoveToTile);

	void UpdateTilesUnderAttack(TArray<FChessTileInfo>& TilesUnderAttack);

	FORCEINLINE bool IsPositionWithinBounds(FVector2D Position) { return Position.X >= 0 && Position.X < 8 && Position.Y >= 0 && Position.Y < 8; }

	UFUNCTION(BlueprintCallable, Category = "+Chess|Piece")
	void PromotePawn(EChessPieceType PromotionType);

#pragma endregion

#pragma region VARIABLES

public:
	float TotalTravelDistance = 0.f;

	FOnPieceCaptured OnPieceCaptured;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece")
	UChessBoardData* ChessBoardData = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Piece")
	FChessPieceInfo ChessPieceInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Piece")
	TArray<FChessTileInfo> ValidMoves;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Piece")
	AChessBoard* ChessBoard = nullptr;

private:
	FPredictProjectilePathParams PredictParams;
	
	TArray<FVector2D> KingMovePositionTileOffsets = { FVector2D(1, -1), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1), FVector2D(-1, 1), FVector2D(-1, 0), FVector2D(-1, -1), FVector2D(0, -1) };

	TArray<FVector2D> KnightMovePositionTileOffsets = { FVector2D(2, -1), FVector2D(2, 1), FVector2D(1, 2), FVector2D(-1, 2), FVector2D(-2, 1), FVector2D(-2, -1), FVector2D(-1, -2), FVector2D(1, -2) };

	TArray<FVector2D> BishopMovePositionDirections = { FVector2D(1, -1), FVector2D(1, 1), FVector2D(-1, 1), FVector2D(-1, -1) };

	TArray<FVector2D> RookMovePositionDirections = { FVector2D(1, 0), FVector2D(0, 1), FVector2D(-1, 0), FVector2D(0, -1) };

	TArray<FVector2D> QueenMovePositionDirections = KingMovePositionTileOffsets;

#pragma endregion
};