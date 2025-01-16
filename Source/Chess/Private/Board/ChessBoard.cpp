// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessBoard.h"

#include "Board/ChessPiece.h"
#include "Board/ChessTile.h"
#include "Core/ChessGameMode.h"
#include "Core/ChessPlayerController.h"
#include "Data/ChessBoardData.h"

#include "Kismet/GameplayStatics.h"

#define PRINTSTRING(Colour, DebugMessage) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, Colour, DebugMessage);

AChessBoard::AChessBoard() :
	ChessTileClass(AChessTile::StaticClass()),
	ChessPieceClass(AChessPiece::StaticClass())
{
	PrimaryActorTick.bCanEverTick = true;

	// DefaultSceneRootComponent
	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRootComponent"));
	SetRootComponent(DefaultSceneRootComponent);

	static ConstructorHelpers::FObjectFinder<UChessBoardData> ChessBoardDataAsset(TEXT("/Script/Chess.ChessBoardData'/Game/+Chess/Data/DA_ChessBoardData.DA_ChessBoardData'"));
	if (ChessBoardDataAsset.Succeeded()) ChessBoardData = ChessBoardDataAsset.Object;
}

void AChessBoard::BeginPlay()
{
	Super::BeginPlay();

	CreateBoard();

	SetupBoard();

	GenerateAllValidMoves(true);
}

void AChessBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChessBoard::CreateBoard()
{
	const float TileSize = 250.f;

	int32 Offset = (TileSize * (static_cast<float>(8) / 2)) - (TileSize / 2);

	for (int32 i = 0; i < 8; i++)
		for (int32 j = 0; j < 8; j++)
			ChessTileLocations.AddUnique(FVector((i * TileSize) - Offset, (j * TileSize) - Offset, 0.f));
	
	// Ensure the board has exactly 64 squares
	ChessBoardLayout.SetNum(64);

	// Clear all squares to empty pieces
	for (FChessTileInfo& Square : ChessBoardLayout)
		Square = FChessTileInfo(); // Initialize as an empty piece

	for (int32 i = 0; i < 64; i++)
	{
		if (AChessTile* Tile = Cast<AChessTile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ChessTileClass, FTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this)))
		{
			Tile->ChessTileInfo.ChessTilePositionIndex = i;
			Tile->ChessTileInfo.bIsWhite = (TileColourAtIndex[i] == 1);

			UGameplayStatics::FinishSpawningActor(Tile, FTransform());

			Tile->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
			Tile->SetActorRelativeLocation(ChessTileLocations[i]);
			ChessTiles.AddUnique(Tile);
			ChessBoardLayout[i] = Tile->ChessTileInfo;
		}
	}
}

void AChessBoard::SetupBoard()
{
#pragma region Initialize Chess Pieces Information
	TArray<FChessPieceInfo> WhiteChessPiecesInfo;
	TArray<FChessPieceInfo> BlackChessPiecesInfo;

	// Pawns
	for (int32 i = 8; i <= 15; i++)	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Pawn, EChessPieceSide::None, i));

	for (int32 i = 48; i <= 55; i++) BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Pawn, EChessPieceSide::None, i));

	// Rooks
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Rook, EChessPieceSide::QueenSide, 0));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Rook, EChessPieceSide::KingSide, 7));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Rook, EChessPieceSide::QueenSide, 56));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Rook, EChessPieceSide::KingSide, 63));

	// Knights
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Knight, EChessPieceSide::None, 1));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Knight, EChessPieceSide::None, 6));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Knight, EChessPieceSide::None, 57));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Knight, EChessPieceSide::None, 62));

	// Bishops
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Bishop, EChessPieceSide::None, 2));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Bishop, EChessPieceSide::None, 5));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Bishop, EChessPieceSide::None, 58));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Bishop, EChessPieceSide::None, 61));

	// Queens
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::Queen, EChessPieceSide::None, 3));
	
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::Queen, EChessPieceSide::None, 59));

	// Kings
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, false, EChessPieceType::King, EChessPieceSide::None, 4));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, false, EChessPieceType::King, EChessPieceSide::None, 60));
#pragma endregion

	// Spawn White Chess Pieces
	for (int32 i = 0; i < ChessBoardData->WhiteChessPiecesInfo.Num(); i++)
		WhiteChessPieces.AddUnique(SpawnChessPiece(/*ChessBoardData->*/WhiteChessPiecesInfo[i]));

	// Spawn Black Chess Pieces
	for (int32 i = 0; i < ChessBoardData->BlackChessPiecesInfo.Num(); i++)
		BlackChessPieces.AddUnique(SpawnChessPiece(/*ChessBoardData->*/BlackChessPiecesInfo[i]));
}

AChessPiece* AChessBoard::SpawnChessPiece(FChessPieceInfo ChessPieceInfo)
{
	AChessPiece* ChessPiece = Cast<AChessPiece>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ChessPieceClass, FTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));
	if (ChessPiece)
	{
		ChessPiece->ChessPieceInfo = ChessPieceInfo;

		UGameplayStatics::FinishSpawningActor(ChessPiece, FTransform());

		ChessPiece->SetActorLocation(ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->GetActorLocation());

		ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->ChessTileInfo.ChessPieceOnTile = ChessPiece;
	}

	return ChessPiece;
}

void AChessBoard::UpdateAttackStatusOfTiles()
{
	// Reset values
	for (AChessTile* Tile : ChessTiles)
	{
		Tile->ChessTileInfo.bIsTileUnderAttackByWhitePiece = false;
		Tile->ChessTileInfo.bIsTileUnderAttackByBlackPiece = false;
	}

	ChessTilesUnderAttackByWhitePieces.Empty();
	ChessTilesUnderAttackByBlackPieces.Empty();

	for (AChessPiece* WhiteChessPiece : WhiteChessPieces)
	{
		if (!WhiteChessPiece) continue;

		WhiteChessPiece->UpdateTilesUnderAttack(ChessTilesUnderAttackByWhitePieces);
	}

	for (FChessTileInfo& TileInfo : ChessTilesUnderAttackByWhitePieces)
		ChessTiles[TileInfo.ChessTilePositionIndex]->ChessTileInfo.bIsTileUnderAttackByWhitePiece =	true;

	for (AChessPiece* BlackChessPiece : BlackChessPieces)
	{
		if (!BlackChessPiece) continue;

		BlackChessPiece->UpdateTilesUnderAttack(ChessTilesUnderAttackByBlackPieces);
	}

	for (FChessTileInfo& TileInfo : ChessTilesUnderAttackByBlackPieces)
		ChessTiles[TileInfo.ChessTilePositionIndex]->ChessTileInfo.bIsTileUnderAttackByBlackPiece = true;

	// Update ChessBoardLayout
	for (int32 i = 0; i < ChessTiles.Num(); i++)
		ChessBoardLayout[i] = ChessTiles[i]->ChessTileInfo;
}

void AChessBoard::ClearAllValidMoves()
{
	for (AChessPiece* WhiteChessPiece : WhiteChessPieces)
		if (WhiteChessPiece) WhiteChessPiece->ValidMoves.Empty();

	for (AChessPiece* BlackChessPiece : BlackChessPieces)
		if (BlackChessPiece) BlackChessPiece->ValidMoves.Empty();
}

void AChessBoard::GenerateAllValidMoves(bool bIsWhiteTurn)
{
	ClearAllValidMoves();

	UpdateAttackStatusOfTiles();

	if (bIsWhiteTurn)
	{
		// Calculate White Pieces Moves
		for (AChessPiece* WhiteChessPiece : WhiteChessPieces)
			if (WhiteChessPiece) WhiteChessPiece->CalculateValidMoves();
	}
	else
	{
		// Calculate Black Pieces Moves
		for (AChessPiece* BlackChessPiece : BlackChessPieces)
			if (BlackChessPiece) BlackChessPiece->CalculateValidMoves();
	}
}

bool AChessBoard::HightlightValidMovesOnTile(bool bHighlight, FChessTileInfo ChessTileInfo)
{
	if (!ChessTileInfo.ChessPieceOnTile)
	{
		PRINTSTRING(FColor::Red, "ChessPieceOnTile is Invalid : ChessBoard.cpp > HighlightValidMovesOnTile()");
		return false;
	}

	if (bHighlight)
	{
		HighlightedTiles = ChessTileInfo.ChessPieceOnTile->ValidMoves;

		if (HighlightedTiles.Num() == 0) return false;

		for (FChessTileInfo& Tile : HighlightedTiles) ChessTiles[Tile.ChessTilePositionIndex]->HighlightTile(true);

	}
	else
	{
		for (FChessTileInfo& Tile : HighlightedTiles) ChessTiles[Tile.ChessTilePositionIndex]->HighlightTile(false);

		HighlightedTiles.Empty();
	}

	return true;
}

void AChessBoard::EnableEnpassant(AChessPiece* EnpassantPiece)
{
	if (!EnpassantPiece) return PRINTSTRING(FColor::Red, "EnpassantPiece Invalid in ChessBoard");

	AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "Invalid PlayerController in ChessTile");

	EnpassantPawn = EnpassantPiece;
	EnpassantTileIndex = EnpassantPawn->ChessPieceInfo.ChessPiecePositionIndex + ((EnpassantPawn->ChessPieceInfo.bIsWhite) ? 8 : -8); // considering piece hasn't moved yet so we take the tile in front of the pawn

	ChessPlayerController->OnPieceMoved.AddDynamic(this, &AChessBoard::DisableEnpassant);
}

void AChessBoard::DisableEnpassant(bool bIsWhite)
{
	if (!EnpassantPawn) return PRINTSTRING(FColor::Red, "EnpassantPiece Invalid in ChessTile");

	if (bIsWhite != EnpassantPawn->ChessPieceInfo.bIsWhite)
	{
		EnpassantPawn = nullptr;
		EnpassantTileIndex = -1;

		AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "Invalid PlayerController in ChessTile");

		ChessPlayerController->OnPieceMoved.RemoveDynamic(this, &AChessBoard::DisableEnpassant);
	}
}

bool AChessBoard::IsKingInCheck(bool bIsWhiteKing, const TArray<FChessTileInfo>& BoardLayout, int32 OutEnpassantTarget) const
{
	// Locate the king
	int32 KingPosition = -1;
	for (int32 i = 0; i < BoardLayout.Num(); i++)
	{
		if (BoardLayout[i].ChessPieceOnTile)
		{
			if (BoardLayout[i].ChessPieceOnTile->ChessPieceInfo.ChessPieceType == EChessPieceType::King && BoardLayout[i].ChessPieceOnTile->ChessPieceInfo.bIsWhite == bIsWhiteKing)
			{
				KingPosition = i;
				break;
			}
		}
	}

	if (KingPosition == -1) {
		// King not found on the board (shouldn't happen in a valid game state)
		return false;
	}

	 //Check for threats from opponent pieces
	for (int32 i = 0; i < BoardLayout.Num(); i++)
	{
		const FChessTileInfo& OpponentPieceTile = BoardLayout[i];

		if (!OpponentPieceTile.ChessPieceOnTile) continue;

		if (OpponentPieceTile.ChessPieceOnTile->ChessPieceInfo.bIsWhite != bIsWhiteKing)
		{
			// Get valid moves for the opponent piece
			TArray<FChessTileInfo> OpponentMoves;

			switch (OpponentPieceTile.ChessPieceOnTile->ChessPieceInfo.ChessPieceType)
			{
			case EChessPieceType::King:
				OpponentMoves = OpponentPieceTile.ChessPieceOnTile->CalculateValidMoveTilesForKing(BoardLayout);
				break;
			case EChessPieceType::Queen:
				OpponentMoves = OpponentPieceTile.ChessPieceOnTile->CalculateValidMoveTilesForQueen(BoardLayout);
				break;
			case EChessPieceType::Bishop:
				OpponentMoves = OpponentPieceTile.ChessPieceOnTile->CalculateValidMoveTilesForBishop(BoardLayout);
				break;
			case EChessPieceType::Knight:
				OpponentMoves = OpponentPieceTile.ChessPieceOnTile->CalculateValidMoveTilesForKnight(BoardLayout);
				break;
			case EChessPieceType::Rook:
				OpponentMoves = OpponentPieceTile.ChessPieceOnTile->CalculateValidMoveTilesForRook(BoardLayout);
				break;
			case EChessPieceType::Pawn:
				OpponentMoves = OpponentPieceTile.ChessPieceOnTile->CalculateValidMoveTilesForPawn(BoardLayout, OutEnpassantTarget);
				break;
			default:
				break;
			}

			// Check if the king's position is in the opponent's moves
			if (OpponentMoves.Contains(BoardLayout[KingPosition]))
				return true; // The king is in check
		}
	}

	return false; // The king is not in check
}