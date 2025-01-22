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

	GenerateAllValidMoves(ChessBoardInfo, true);
}

void AChessBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChessBoard::CreateBoard()
{
	TArray<uint8> TileColourAtIndex = { 0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0 };

	const float TileSize = 250.f;

	int32 Offset = (TileSize * (static_cast<float>(8) / 2)) - (TileSize / 2);

	for (int32 i = 0; i < 8; i++)
		for (int32 j = 0; j < 8; j++)
			ChessTileLocations.AddUnique(FVector((i * TileSize) - Offset, (j * TileSize) - Offset, 0.f));
	
	// Ensure the board has exactly 64 squares
	ChessBoardInfo.TilesInfo.SetNum(64);

	// Clear all squares to empty pieces
	for (FChessTileInfo& Square : ChessBoardInfo.TilesInfo)
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
			ChessBoardInfo.TilesInfo[i] = Tile->ChessTileInfo;
		}
	}
}

void AChessBoard::SetupBoard()
{
#pragma region Initialize Chess Pieces Information
	TArray<FChessPieceInfo> WhiteChessPiecesInfo;
	TArray<FChessPieceInfo> BlackChessPiecesInfo;

	// Pawns
	for (int32 i = 8; i <= 15; i++)	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Pawn, EChessPieceSide::None, i, TArray<int32>()));

	for (int32 i = 48; i <= 55; i++) BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Pawn, EChessPieceSide::None, i, TArray<int32>()));

	// Rooks
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Rook, EChessPieceSide::QueenSide, 0, TArray<int32>()));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Rook, EChessPieceSide::KingSide, 7, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Rook, EChessPieceSide::QueenSide, 56, TArray<int32>()));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Rook, EChessPieceSide::KingSide, 63, TArray<int32>()));

	// Knights
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Knight, EChessPieceSide::None, 1, TArray<int32>()));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Knight, EChessPieceSide::None, 6, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Knight, EChessPieceSide::None, 57, TArray<int32>()));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Knight, EChessPieceSide::None, 62, TArray<int32>()));

	// Bishops
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Bishop, EChessPieceSide::None, 2, TArray<int32>()));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Bishop, EChessPieceSide::None, 5, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Bishop, EChessPieceSide::None, 58, TArray<int32>()));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Bishop, EChessPieceSide::None, 61, TArray<int32>()));

	// Queens
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Queen, EChessPieceSide::None, 3, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Queen, EChessPieceSide::None, 59, TArray<int32>()));

	// Kings
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::King, EChessPieceSide::None, 4, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::King, EChessPieceSide::None, 60, TArray<int32>()));
#pragma endregion

	// Spawn White Chess Pieces
	for (int32 i = 0; i < WhiteChessPiecesInfo.Num(); i++)
	{
		WhiteChessPieces.AddUnique(SpawnChessPiece(WhiteChessPiecesInfo[i]));
	}

	// Spawn Black Chess Pieces
	for (int32 i = 0; i < BlackChessPiecesInfo.Num(); i++)
	{
		BlackChessPieces.AddUnique(SpawnChessPiece(BlackChessPiecesInfo[i]));
	}
}

AChessPiece* AChessBoard::SpawnChessPiece(FChessPieceInfo ChessPieceInfo)
{
	AChessPiece* ChessPiece = Cast<AChessPiece>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ChessPieceClass, FTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));
	if (ChessPiece)
	{
		ChessPiece->ChessPieceInfo = ChessPieceInfo;

		UGameplayStatics::FinishSpawningActor(ChessPiece, FTransform());

		ChessPiece->SetActorLocation(ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->GetActorLocation());

		ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->ChessTileInfo.ChessPieceOnTile = ChessPieceInfo;
		ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->ChessPieceOnTile = ChessPiece;

		ChessBoardInfo.TilesInfo[ChessPieceInfo.ChessPiecePositionIndex].ChessPieceOnTile = ChessPieceInfo;

		//if (ChessPieceInfo.bIsWhite)
		//{
		//	ChessBoardInfo.WhitePiecesInfo.AddUnique(ChessPieceInfo);
		//}
		//else
		//{
		//	ChessBoardInfo.BlackPiecesInfo.AddUnique(ChessPieceInfo);
		//}
	}

	return ChessPiece;
}

bool AChessBoard::HightlightValidMovesOnTile(bool bHighlight, int32 TileIndex)
{
	if (ChessBoardInfo.TilesInfo[TileIndex].ChessPieceOnTile.ChessPiecePositionIndex == -1)
	{
		PRINTSTRING(FColor::Red, "ChessPieceOnTile is Invalid : ChessBoard.cpp > HighlightValidMovesOnTile()");
		return false;
	}

	if (bHighlight)
	{
		for (int32 ValidMove : ChessBoardInfo.TilesInfo[TileIndex].ChessPieceOnTile.ValidMoves) HighlightedTiles.AddUnique(ChessTiles[ValidMove]->ChessTileInfo);

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

void AChessBoard::GenerateAllValidMoves(FChessBoardInfo& BoardInfo, bool bIsWhiteTurn)
{
	ClearAllValidMoves(BoardInfo);

	UpdateAttackStatusOfTiles(BoardInfo);

	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1) // if theres a piece on the tile
			if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn) // if piece colour is same as current player turn
				CalculateValidMovesForPiece(BoardInfo, Tile.ChessPieceOnTile);

	//if (bIsWhiteTurn)
	//{
	//	// Calculate White Pieces Moves
	//	for (FChessPieceInfo& WhitePiece : BoardInfo.WhitePiecesInfo)
	//	{
	//		if (WhitePiece.ChessPiecePositionIndex > -1)
	//		{
	//			CalculateValidMovesForPiece(BoardInfo, WhitePiece);
	//			BoardInfo.TilesInfo[WhitePiece.ChessPiecePositionIndex].ChessPieceOnTile = WhitePiece;
	//		}
	//	}
	//}
	//else
	//{
	//	// Calculate Black Pieces Moves
	//	for (FChessPieceInfo& BlackPiece : BoardInfo.BlackPiecesInfo)
	//	{
	//		if (BlackPiece.ChessPiecePositionIndex > -1)
	//		{
	//			CalculateValidMovesForPiece(BoardInfo, BlackPiece);
	//			BoardInfo.TilesInfo[BlackPiece.ChessPiecePositionIndex].ChessPieceOnTile = BlackPiece;
	//		}
	//	}
	//}
}

void AChessBoard::ClearAllValidMoves(FChessBoardInfo& BoardInfo)
{
	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1) // if theres a piece on the tile
		{
			Tile.ChessPieceOnTile.ValidMoves.Empty();
		}
	}

	//for (FChessPieceInfo& WhitePiece : BoardInfo.WhitePiecesInfo)
	//{
	//	if (WhitePiece.ChessPiecePositionIndex > -1)
	//	{
	//		WhitePiece.ValidMoves.Empty();
	//		BoardInfo.TilesInfo[WhitePiece.ChessPiecePositionIndex].ChessPieceOnTile.ValidMoves.Empty();
	//	}
	//}
	//
	//for (FChessPieceInfo& BlackPiece : BoardInfo.BlackPiecesInfo)
	//{
	//	if (BlackPiece.ChessPiecePositionIndex > -1)
	//	{
	//		BlackPiece.ValidMoves.Empty();
	//		BoardInfo.TilesInfo[BlackPiece.ChessPiecePositionIndex].ChessPieceOnTile.ValidMoves.Empty();
	//	}
	//}
}

void AChessBoard::UpdateAttackStatusOfTiles(FChessBoardInfo& BoardInfo)
{
	// Reset values
	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		Tile.bIsTileUnderAttackByWhitePiece = false;
		Tile.bIsTileUnderAttackByBlackPiece = false;
	}

	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1) // if theres a piece on the tile
			UpdateTilesUnderAttackByPiece(BoardInfo, Tile.ChessPieceOnTile);

	//for (FChessPieceInfo& WhitePiece : BoardInfo.WhitePiecesInfo)
	//	if (WhitePiece.ChessPiecePositionIndex > -1) UpdateTilesUnderAttackByPiece(BoardInfo, WhitePiece);

	//for (FChessPieceInfo& BlackPiece : BoardInfo.BlackPiecesInfo)
	//	if (BlackPiece.ChessPiecePositionIndex > -1) UpdateTilesUnderAttackByPiece(BoardInfo, BlackPiece);
}

void AChessBoard::UpdateTilesUnderAttackByPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<int32> ValidMovePositions;

	switch (Piece.ChessPieceType)
	{
	case EChessPieceType::King:
		for (FVector2D& Position : KingMovePositionTileOffsets)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// ignore tile if friendly piece is on that tile
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
						continue;

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
		break;
	case EChessPieceType::Queen:
	{
		for (FVector2D& Position : QueenMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

					// if a piece is on that tile...
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					{
						// and if its a friendly piece
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePositionIndex);

						// if it not opponents king
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePositionIndex);
				}
			}
		}
		break;
	}
	case EChessPieceType::Bishop:
	{
		for (FVector2D& Position : BishopMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

					// if a piece is on that tile...
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					{
						// and if its a friendly piece
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePositionIndex);

						// if it not opponents king
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePositionIndex);
				}
			}
		}
		break;
	}
	case EChessPieceType::Knight:
	{
		for (FVector2D& Position : KnightMovePositionTileOffsets)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// ignore tile if friendly piece is on that tile
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
						continue;

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
		break;
	}
	case EChessPieceType::Rook:
	{
		for (FVector2D& Position : RookMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

					// if a piece is on that tile...
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					{
						// and if its a friendly piece
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePositionIndex);

						// if it not opponents king
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePositionIndex);
				}
			}
		}
		break;
	}
	case EChessPieceType::Pawn:
	{
		if (Piece.bIsWhite)
		{
			// Diagonal Captures
			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 - 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 - 1);
				}
			}

			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
				}
			}

			// En Passant White -> Black
			if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 - 1);

			if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
		}
		else
		{
			// Diagonal Captures
			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 - 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 - 1);
				}
			}

			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 + 1);
				}
			}

			// En Passant Black -> White
			if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 - 1);

			if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 + 1);
		}
		break;
	}
	default:
		break;
	}

	for (int32 Position : ValidMovePositions)
	{
		if (Piece.bIsWhite)
			BoardInfo.TilesInfo[Position].bIsTileUnderAttackByWhitePiece = true;
		else
			BoardInfo.TilesInfo[Position].bIsTileUnderAttackByBlackPiece = true;
	}
}

void AChessBoard::CalculateValidMovesForPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidMovesBeforeFilteringForCheck;

	switch (Piece.ChessPieceType)
	{
		case EChessPieceType::King:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForKing(BoardInfo, Piece);
			break;
		case EChessPieceType::Queen:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForQueen(BoardInfo, Piece);
			break;
		case EChessPieceType::Bishop:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForBishop(BoardInfo, Piece);
			break;
		case EChessPieceType::Knight:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForKnight(BoardInfo, Piece);
			break;
		case EChessPieceType::Rook:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForRook(BoardInfo, Piece);
			break;
		case EChessPieceType::Pawn:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForPawn(BoardInfo, Piece);
			break;
		default:
			break;
	}

	for (FChessTileInfo& ValidTile : FilterMovesForCheck(BoardInfo, Piece, ValidMovesBeforeFilteringForCheck))
		Piece.ValidMoves.Add(ValidTile.ChessTilePositionIndex);
		
	BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex].ChessPieceOnTile = Piece;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForKing(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidTiles;
		
	// One Tile Omnidirectional moves
	for (FVector2D& Position : KingMovePositionTileOffsets)
	{
		FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
		
		if (IsPositionWithinBounds(RelativePosition))
		{
			int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;
		
			// ignore tile if friendly piece is on that tile
			if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
					continue;
		
			// ignore tile if its is under attack by enemy piece
			if ((Piece.bIsWhite)
				? BoardInfo.TilesInfo[RelativePositionIndex].bIsTileUnderAttackByBlackPiece
				: BoardInfo.TilesInfo[RelativePositionIndex].bIsTileUnderAttackByWhitePiece)
				continue;
		
			ValidTiles.Add(BoardInfo.TilesInfo[RelativePositionIndex]);
		}
	}
		
	// Castling Moves if available
	if (Piece.bIsWhite)
	{
		// if White king is not under check
		if (!BoardInfo.bIsWhiteKingUnderCheck)
		{
			// if White king side rook alive
			if (BoardInfo.bIsWhiteKingSideRookAlive)
				// if white king and king side rook have not moved
				if (!BoardInfo.HasWhiteKingOrKingSideRookMoved())
					// if no piece is present one tile to the right
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the right isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].bIsTileUnderAttackByBlackPiece)
							// if no piece is present two tiles to the right
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the right isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].bIsTileUnderAttackByBlackPiece)
									ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2]);
		
			// if White queen side rook alive
			if (BoardInfo.bIsWhiteQueenSideRookAlive)
				// if white king and queen side rook have not moved
				if (!BoardInfo.HasWhiteKingOrQueenSideRookMoved())
					// if no piece is present one tile to the left
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the left isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].bIsTileUnderAttackByBlackPiece)
							// if no piece is present two tiles to the left
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the left isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].bIsTileUnderAttackByBlackPiece)
									// if no piece is present three tiles to the left
									if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 3].ChessPieceOnTile.ChessPiecePositionIndex == -1)
										ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2]);
		}
	}
	else
	{
		// if Black king is not under check
		if (!BoardInfo.bIsBlackKingUnderCheck)
		{
			// if black king side rook alive
			if (BoardInfo.bIsBlackKingSideRookAlive)
				// if black king and king side rook have not moved
				if (!BoardInfo.HasBlackKingOrKingSideRookMoved())
					// if no piece is present one tile to the right
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the right isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].bIsTileUnderAttackByWhitePiece)
							// if no piece is present two tiles to the right
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the right isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].bIsTileUnderAttackByWhitePiece)
									ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2]);
		
			// if black queen side rook alive
			if (BoardInfo.bIsBlackQueenSideRookAlive)
				// if black king and queen side rook have not moved
				if (!BoardInfo.HasBlackKingOrQueenSideRookMoved())
					// if no piece is present one tile to the left
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the left isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].bIsTileUnderAttackByWhitePiece)
							// if no piece is present two tiles to the left
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the left isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].bIsTileUnderAttackByWhitePiece)
									// if no piece is present three tiles to the left
									if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 3].ChessPieceOnTile.ChessPiecePositionIndex == -1)
										ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2]);
		}
	}
		
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForQueen(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidTiles;
		
	for (FVector2D& Position : QueenMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;
		
				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
		
					break;
				}
		
				ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}
			
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForBishop(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidTiles;

	for (FVector2D& Position : BishopMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);

					break;
				}

				ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}

	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForKnight(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidTiles;
		
	for (FVector2D& Position : KnightMovePositionTileOffsets)
	{
		FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
		if (IsPositionWithinBounds(RelativePosition))
		{
			int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;
	
			// ignore tile if friendly piece is on that tile
			if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
					continue;

			ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
		}
	}
		
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForRook(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidTiles;

	for (FVector2D& Position : RookMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);

					break;
				}

				ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}

	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForPawn(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<FChessTileInfo> ValidTiles;
		
	if (Piece.bIsWhite)
	{
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 0)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
				ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8]);
	
		if (Piece.GetChessPiecePositionFromIndex().X == 1 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(2, 0)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 16].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in two tiles front
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 16]);
	
		// Diagonal Captures
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1]);

		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1]);
	
		// En Passant White -> Black
		if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1]);
	
		if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1]);
	}
	else
	{
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 0)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
				ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8]);
	
		if (Piece.GetChessPiecePositionFromIndex().X == 6 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-2, 0)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 16].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in two tiles front
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 16]);
	
		// Diagonal Captures
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1]);
	
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1]);
	
		// En Passant Black -> White
		if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1]);
	
		if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1]);
	}
	
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::FilterMovesForCheck(FChessBoardInfo Board, FChessPieceInfo& Piece, TArray<FChessTileInfo> ValidMovesBeforeFilteration)
{
	TArray<FChessTileInfo> ValidMovesAfterFilteration;

	for (FChessTileInfo ValidMove : ValidMovesBeforeFilteration)
	{
		FChessBoardInfo SimulatedBoard = Board;

		SimulateMove(SimulatedBoard, Piece, ValidMove.ChessTilePositionIndex);
		
		if (!IsKingInCheck(SimulatedBoard, Piece.bIsWhite)) ValidMovesAfterFilteration.AddUnique(ValidMove);
	}

	return ValidMovesAfterFilteration;
}

void AChessBoard::SimulateMove(FChessBoardInfo& BoardInfo, FChessPieceInfo Piece, int32 ToPosition)
{
	FChessTileInfo& FromTile = BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex];
	FChessTileInfo& ToTile = BoardInfo.TilesInfo[ToPosition];

	// Move the piece to the target tile
	ToTile.ChessPieceOnTile = FromTile.ChessPieceOnTile;
	FromTile.ChessPieceOnTile = FChessPieceInfo(); // Clear the original tile

	// Update Piece Position
	ToTile.ChessPieceOnTile.ChessPiecePositionIndex = ToTile.ChessTilePositionIndex;

	switch (Piece.ChessPieceType)
	{
	case EChessPieceType::King:
		// Handle castling (if the king moves two tiles)
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 2)
		{
			if (ToPosition > Piece.ChessPiecePositionIndex)
			{
				// King-side castling
				SimulateMove(BoardInfo, BoardInfo.TilesInfo[ToPosition + 1].ChessPieceOnTile, ToPosition - 1);
			}
			else
			{
				// Queen-side castling
				SimulateMove(BoardInfo, BoardInfo.TilesInfo[ToPosition - 2].ChessPieceOnTile, ToPosition + 1);
			}
		}

		if (Piece.bIsWhite)
		{
			if (!BoardInfo.bHasWhiteKingMoved) BoardInfo.bHasWhiteKingMoved = true;
		}
		else
		{
			if (!BoardInfo.bHasBlackKingMoved) BoardInfo.bHasBlackKingMoved = true;
		}
		break;
	case EChessPieceType::Queen:
		break;
	case EChessPieceType::Bishop:
		break;
	case EChessPieceType::Knight:
		break;
	case EChessPieceType::Rook:
		if (Piece.bIsWhite)
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!BoardInfo.bHasWhiteKingSideRookMoved) BoardInfo.bHasWhiteKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!BoardInfo.bHasWhiteQueenSideRookMoved) BoardInfo.bHasWhiteQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!BoardInfo.bHasBlackKingSideRookMoved) BoardInfo.bHasBlackKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!BoardInfo.bHasBlackQueenSideRookMoved) BoardInfo.bHasBlackQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		break;
	case EChessPieceType::Pawn:
		// Handle enpassant captures
		if (ToPosition == BoardInfo.EnpassantTileIndex)
		{
			BoardInfo.TilesInfo[BoardInfo.EnpassantPawn.ChessPiecePositionIndex].ChessPieceOnTile = FChessPieceInfo(); // Remove the captured pawn
		}

		// Update enpassant target
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 16)
		{
			// Set the enpassant target to the tile behind the pawn
			BoardInfo.EnpassantTileIndex = ToTile.ChessPieceOnTile.bIsWhite ? ToTile.ChessPieceOnTile.ChessPiecePositionIndex - 8 : ToTile.ChessPieceOnTile.ChessPiecePositionIndex + 8;
			BoardInfo.EnpassantPawn = ToTile.ChessPieceOnTile;
		}
		else
		{
			// Clear the enpassant target
			BoardInfo.EnpassantTileIndex = -1;
			BoardInfo.EnpassantPawn = FChessPieceInfo();
		}

		// Handle pawn promotion (if a pawn reaches the opposite end of the board)
		if ((ToPosition < 8 || ToPosition >= 56))
		{
			// Promote to a queen (default behavior)
			ToTile.ChessPieceOnTile.ChessPieceType = EChessPieceType::Queen;
		}
		break;
	default:
		break;
	}

	/*FChessTileInfo& FromTile = BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex];
	FChessTileInfo& ToTile = BoardInfo.TilesInfo[ToPosition];

	// Move the piece to the target tile
	ToTile.ChessPieceOnTile = FromTile.ChessPieceOnTile;
	FromTile.ChessPieceOnTile = FChessPieceInfo(); // Clear the original tile

	// Handle en passant captures
	if (Piece.ChessPieceType == EChessPieceType::Pawn && ToPosition == BoardInfo.EnpassantTileIndex)
	{
		// Determine the captured pawn's position
		int32 CapturedPawnPosition = Piece.bIsWhite ? ToPosition - 8 : ToPosition + 8;
		BoardInfo.TilesInfo[CapturedPawnPosition] = FChessTileInfo(); // Remove the captured pawn
	}

	// Update en passant target
	if (Piece.ChessPieceType == EChessPieceType::Pawn && FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 16)
	{
		// Set the en passant target to the square behind the pawn
		BoardInfo.EnpassantTileIndex = Piece.bIsWhite ? Piece.ChessPiecePositionIndex + 8 : Piece.ChessPiecePositionIndex - 8;
		BoardInfo.EnpassantPawn = Piece;
	}
	else
	{
		// Clear the en passant target
		BoardInfo.EnpassantTileIndex = -1;
		BoardInfo.EnpassantPawn = FChessPieceInfo();
	}

	// Handle castling (if the king moves two squares)
	if (Piece.ChessPieceType == EChessPieceType::King && FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 2)
	{
		if (ToPosition > Piece.ChessPiecePositionIndex)
		{
			// King-side castling
			BoardInfo.TilesInfo[ToPosition - 1] = BoardInfo.TilesInfo[ToPosition + 1]; // Move the rook
			BoardInfo.TilesInfo[ToPosition + 1] = FChessTileInfo(); // Clear the original rook square
		}
		else
		{
			// Queen-side castling
			BoardInfo.TilesInfo[ToPosition + 1] = BoardInfo.TilesInfo[ToPosition - 2]; // Move the rook
			BoardInfo.TilesInfo[ToPosition - 2] = FChessTileInfo(); // Clear the original rook square
		}
	}

	// Handle pawn promotion (if a pawn reaches the opposite end of the board)
	if (Piece.ChessPieceType == EChessPieceType::Pawn && (ToPosition < 8 || ToPosition >= 56))
	{
		// Promote to a queen (default behavior)
		Piece.ChessPieceType = EChessPieceType::Queen;
	}*/
}

void AChessBoard::MakeMove(AChessTile* StartTile, AChessTile* EndTile)
{
	if (!StartTile) return PRINTSTRING(FColor::Red, "StartTile is INVALID in ChessBoard");
	if (!EndTile) return PRINTSTRING(FColor::Red, "EndTile is INVALID in ChessBoard");

	AChessPiece* ChessPiece = Cast<AChessPiece>(StartTile->ChessPieceOnTile);
	if (!ChessPiece) return PRINTSTRING(FColor::Red, "ChessPiece is INVALID in ChessBoard");

	FChessPieceInfo Piece = StartTile->ChessPieceOnTile->ChessPieceInfo;
	int32 ToPosition = EndTile->ChessTileInfo.ChessTilePositionIndex;

	FChessTileInfo& FromTile = ChessBoardInfo.TilesInfo[Piece.ChessPiecePositionIndex];
	FChessTileInfo& ToTile = ChessBoardInfo.TilesInfo[ToPosition];

	// Move the piece to the target tile
	ChessPiece->MovePiece(EndTile);

	// Update ChessPieceOnTile
	ToTile.ChessPieceOnTile = FromTile.ChessPieceOnTile;
	FromTile.ChessPieceOnTile = FChessPieceInfo(); // Clear the original tile

	EndTile->ChessPieceOnTile = StartTile->ChessPieceOnTile;
	StartTile->ChessPieceOnTile = nullptr;

	// Update Piece
	ToTile.ChessPieceOnTile.ChessPiecePositionIndex = ToTile.ChessTilePositionIndex;

	ChessPiece->ChessPieceInfo = ToTile.ChessPieceOnTile;

	// Update Tile Infos
	StartTile->ChessTileInfo = FromTile;
	EndTile->ChessTileInfo = ToTile;

	switch (Piece.ChessPieceType)
	{
	case EChessPieceType::King:
		// Handle castling (if the king moves two tiles)
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 2)
		{
			if (ToPosition > Piece.ChessPiecePositionIndex)
			{
				// King-side castling
				MakeMove(ChessTiles[ToPosition + 1], ChessTiles[ToPosition - 1]);
			}
			else
			{
				// Queen-side castling
				MakeMove(ChessTiles[ToPosition - 2], ChessTiles[ToPosition + 1]);
			}
		}

		if (Piece.bIsWhite)
		{
			if (!ChessBoardInfo.bHasWhiteKingMoved) ChessBoardInfo.bHasWhiteKingMoved = true;
		}
		else
		{
			if (!ChessBoardInfo.bHasBlackKingMoved) ChessBoardInfo.bHasBlackKingMoved = true;
		}
		break;
	case EChessPieceType::Queen:
		break;
	case EChessPieceType::Bishop:
		break;
	case EChessPieceType::Knight:
		break;
	case EChessPieceType::Rook:
		if (Piece.bIsWhite)
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoardInfo.bHasWhiteKingSideRookMoved) ChessBoardInfo.bHasWhiteKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoardInfo.bHasWhiteQueenSideRookMoved) ChessBoardInfo.bHasWhiteQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoardInfo.bHasBlackKingSideRookMoved) ChessBoardInfo.bHasBlackKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoardInfo.bHasBlackQueenSideRookMoved) ChessBoardInfo.bHasBlackQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		break;
	case EChessPieceType::Pawn:
		// Handle enpassant captures
		if (ToPosition == ChessBoardInfo.EnpassantTileIndex && EnpassantPawn)
		{
			ChessBoardInfo.TilesInfo[ChessBoardInfo.EnpassantPawn.ChessPiecePositionIndex].ChessPieceOnTile = FChessPieceInfo();

			if (AChessTile* EnpassantPawnTile = ChessTiles[EnpassantPawn->ChessPieceInfo.ChessPiecePositionIndex]) EnpassantPawnTile->ChessPieceOnTile = nullptr;

			EnpassantPawn->CapturePiece();
		}

		// Update enpassant target
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 16)
		{
			// Set the enpassant target to the tile behind the pawn
			//ChessBoardInfo.EnpassantTileIndex = ToTile.ChessPieceOnTile.bIsWhite ? ToTile.ChessPieceOnTile.ChessPiecePositionIndex - 8 : ToTile.ChessPieceOnTile.ChessPiecePositionIndex + 8;
			//ChessBoardInfo.EnpassantPawn = ToTile.ChessPieceOnTile;

			EnableEnpassant(ChessPiece);
		}
		else
		{
			// Clear the enpassant target
			//ChessBoardInfo.EnpassantTileIndex = -1;
			//ChessBoardInfo.EnpassantPawn = FChessPieceInfo();

			DisableEnpassant(ChessPiece->ChessPieceInfo.bIsWhite);
		}

		// Handle pawn promotion (if a pawn reaches the opposite end of the board)
		if ((ToPosition < 8 || ToPosition >= 56))
		{
			AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (ChessPlayerController)
			{
				ChessPlayerController->SpawnPawnPromotionUI(ChessPiece);
			}
			else
			{
				PRINTSTRING(FColor::Red, "ChessPlayerController is INVALID in ChessBoard");

				// Promote to a queen (default behavior)
				ChessPiece->PromotePawn(EChessPieceType::Queen);
			}
		}
		break;
	default:
		break;
	}

	/*
	switch (ChessPieceInfo.ChessPieceType)
	{
	case EChessPieceType::King:
		// if King hasn't moved
		if (!ChessPieceInfo.bHasMoved)
		{
			// if first move is two tiles away
			if (FMath::Abs(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y - ChessPieceInfo.GetChessPiecePositionFromIndex().Y) == 2)
			{
				// King Side Castling
				if (MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y > ChessPieceInfo.GetChessPiecePositionFromIndex().Y)
				{
					// if king side rook piece
					if (AChessPiece* KingSideRook = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 7))->ChessPieceOnTile)
					{
						// and if king side rook hasn't moved
						if (!KingSideRook->ChessPieceInfo.bHasMoved)
						{
							// and if the rook is same colour as king
							if (KingSideRook->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
							{
								// and if it is actually a rook and it is in fact on the king side
								if (KingSideRook->ChessPieceInfo.ChessPieceType == EChessPieceType::Rook && KingSideRook->ChessPieceInfo.ChessPieceSide == EChessPieceSide::KingSide)
								{
									// then get the king side rook castling tile
									if (AChessTile* KingSideRookCastlingTile = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 5)))
									{
										// and if no piece is on that tile
										if (KingSideRookCastlingTile->ChessPieceOnTile == nullptr)
										{
											// move the rook to king side rook castling tile
											KingSideRook->MovePiece(KingSideRookCastlingTile);

											MoveToTile->ChessPieceOnTile = KingSideRook;
											KingSideRook->ChessPieceInfo.ChessPiecePositionIndex = MoveToTile->ChessTileInfo.ChessTilePositionIndex;

											ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 7))->ChessPieceOnTile = nullptr;

											PRINTSTRING(FColor::Green, "King Side Castling");
										}
										else
										{
											PRINTSTRING(FColor::Red, "KingSideRookCastlingTile is not empty");
										}
									}
									else
									{
										PRINTSTRING(FColor::Red, "KingSideRookCastlingTile is invalid");
									}
								}
								else
								{
									PRINTSTRING(FColor::Red, "Not a rook or not a king side rook");
								}
							}
							else
							{
								PRINTSTRING(FColor::Red, "Rook is not of the same colour as King");
							}
						}
						else
						{
							PRINTSTRING(FColor::Red, "KingSideRook has moved");
						}
					}
					else
					{
						PRINTSTRING(FColor::Red, "KingSideRook Invalid");
					}
				}
				else // Queen Side Castling
				{
					// if queen side rook piece
					if (AChessPiece* QueenSideRook = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 0))->ChessPieceOnTile)
					{
						// and if king side rook hasn't moved
						if (!QueenSideRook->ChessPieceInfo.bHasMoved)
						{
							// and if the rook is same colour as king
							if (QueenSideRook->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
							{
								// and if it is actually a rook and it is in fact on the queen side
								if (QueenSideRook->ChessPieceInfo.ChessPieceType == EChessPieceType::Rook && QueenSideRook->ChessPieceInfo.ChessPieceSide == EChessPieceSide::QueenSide)
								{
									// then get the queen side rook castling tile
									if (AChessTile* QueenSideRookCastlingTile = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 3)))
									{
										// and if no piece is on that tile
										if (QueenSideRookCastlingTile->ChessPieceOnTile == nullptr)
										{
											// move the rook to queen side rook castling tile
											QueenSideRook->MovePiece(QueenSideRookCastlingTile);

											MoveToTile->ChessPieceOnTile = QueenSideRook;
											QueenSideRook->ChessPieceInfo.ChessPiecePositionIndex = MoveToTile->ChessTileInfo.ChessTilePositionIndex;

											ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 0))->ChessPieceOnTile = nullptr;

											PRINTSTRING(FColor::Green, "Queen Side Castling");
										}
										else
										{
											PRINTSTRING(FColor::Red, "QueenSideRookCastlingTile is not empty");
										}
									}
									else
									{
										PRINTSTRING(FColor::Red, "QueenSideRookCastlingTile is invalid");
									}
								}
								else
								{
									PRINTSTRING(FColor::Red, "Not a rook or not a queen side rook");
								}
							}
							else
							{
								PRINTSTRING(FColor::Red, "Rook is not of the same colour as King");
							}
						}
						else
						{
							PRINTSTRING(FColor::Red, "QueenSideRook has moved");
						}
					}
					else
					{
						PRINTSTRING(FColor::Red, "QueenSideRook Invalid");
					}
				}
			}
		}
		else
		{
			PRINTSTRING(FColor::Red, "King has Moved");
		}

		if (ChessPieceInfo.bIsWhite)
		{
			if (!ChessBoard->ChessBoardInfo.bHasWhiteKingMoved) ChessBoard->ChessBoardInfo.bHasWhiteKingMoved = true;
		}
		else
		{
			if (!ChessBoard->ChessBoardInfo.bHasBlackKingMoved) ChessBoard->ChessBoardInfo.bHasBlackKingMoved = true;
		}
		break;
	case EChessPieceType::Queen:
		break;
	case EChessPieceType::Bishop:
		break;
	case EChessPieceType::Knight:
		break;
	case EChessPieceType::Rook:
		if (ChessPieceInfo.bIsWhite)
		{
			switch (ChessPieceInfo.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoard->ChessBoardInfo.bHasWhiteKingSideRookMoved) ChessBoard->ChessBoardInfo.bHasWhiteKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoard->ChessBoardInfo.bHasWhiteQueenSideRookMoved) ChessBoard->ChessBoardInfo.bHasWhiteQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (ChessPieceInfo.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoard->ChessBoardInfo.bHasBlackKingSideRookMoved) ChessBoard->ChessBoardInfo.bHasBlackKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoard->ChessBoardInfo.bHasBlackQueenSideRookMoved) ChessBoard->ChessBoardInfo.bHasBlackQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		break;
	case EChessPieceType::Pawn:
		if (!ChessPieceInfo.bHasMoved) // is first move of pawn
		{
			// if first move is two tiles away
			if (FMath::Abs(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X - ChessPieceInfo.GetChessPiecePositionFromIndex().X) == 2)
			{
				bool bEnableEnpassant = false;

				for (int32 i = -1; i <= 1; i += 2) // two sides of piece
				{
					if (AChessTile* ChessTileOnSide = ChessBoard->GetChessTileAtPosition(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex() + FVector2D(0, i)))
					{
						if (ChessTileOnSide->ChessPieceOnTile) // if theres a piece on the side...
						{
							// and if the side piece is an opponent pawn
							if (ChessTileOnSide->ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite && ChessTileOnSide->ChessPieceOnTile->ChessPieceInfo.ChessPieceType == EChessPieceType::Pawn)
							{
								bEnableEnpassant = true;
							}
						}
					}
				}

				if (bEnableEnpassant) ChessBoard->EnableEnpassant(ChessBoard->ChessBoardInfo, ChessPieceInfo);
			}
		}
		else if (MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X == 0 || MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X == 7) // Pawn has reached the end of the line
		{
			AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (ChessPlayerController)
			{
				ChessPlayerController->SpawnPawnPromotionUI(this);
			}
			else
			{
				PRINTSTRING(FColor::Red, "ChessPlayerController is INVALID in ChessPiece");
				PromotePawn(EChessPieceType::Queen); // fallback promotion if UI doesn't spawn
			}
		}
		else if (MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y == (ChessPieceInfo.GetChessPiecePositionFromIndex().Y + 1) || MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y == (ChessPieceInfo.GetChessPiecePositionFromIndex().Y - 1)) // is a diagonal move
		{
			// TODO : FIX ENPASSANT ENABLE AND DISABLE
			if (MoveToTile->ChessTileInfo.ChessTilePositionIndex == ChessBoard->ChessBoardInfo.EnpassantTileIndex && ChessBoard->EnpassantPawn) // Enpassant Capture
			{
				if (AChessTile* EnpassantPawnOnTile = ChessBoard->ChessTiles[ChessBoard->EnpassantPawn->ChessPieceInfo.ChessPiecePositionIndex])
				{
					EnpassantPawnOnTile->ChessPieceOnTile = nullptr;
					ChessBoard->EnpassantPawn->CapturePiece();
					ChessBoard->DisableEnpassant(ChessBoard->ChessBoardInfo, ChessPieceInfo.bIsWhite);
				}
			}
		}
		break;
	default:
		break;
	}
	*/
}

bool AChessBoard::IsKingInCheck(const FChessBoardInfo& BoardInfo, bool bIsWhiteKing)
{
	// Locate the king
	int32 KingPosition = -1;
	for (int32 i = 0; i < BoardInfo.TilesInfo.Num(); i++)
	{
		if (BoardInfo.TilesInfo[i].ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			if (BoardInfo.TilesInfo[i].ChessPieceOnTile.ChessPieceType == EChessPieceType::King && BoardInfo.TilesInfo[i].ChessPieceOnTile.bIsWhite == bIsWhiteKing)
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
	for (int32 i = 0; i < BoardInfo.TilesInfo.Num(); i++)
	{
		FChessTileInfo OpponentPieceTile = BoardInfo.TilesInfo[i];

		if (OpponentPieceTile.ChessPieceOnTile.ChessPiecePositionIndex == -1) continue;

		if (OpponentPieceTile.ChessPieceOnTile.bIsWhite != bIsWhiteKing)
		{
			// Get valid moves for the opponent piece
			TArray<FChessTileInfo> OpponentMoves;

			switch (OpponentPieceTile.ChessPieceOnTile.ChessPieceType)
			{
			case EChessPieceType::King:
				OpponentMoves = CalculateValidMoveTilesForKing(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Queen:
				OpponentMoves = CalculateValidMoveTilesForQueen(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Bishop:
				OpponentMoves = CalculateValidMoveTilesForBishop(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Knight:
				OpponentMoves = CalculateValidMoveTilesForKnight(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Rook:
				OpponentMoves = CalculateValidMoveTilesForRook(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Pawn:
				OpponentMoves = CalculateValidMoveTilesForPawn(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			default:
				break;
			}

			// Check if the king's position is in the opponent's moves
			if (OpponentMoves.Contains(BoardInfo.TilesInfo[KingPosition]))
				return true; // The king is in check
		}
	}

	return false; // The king is not in check
}

void AChessBoard::EnableEnpassant(AChessPiece* EnpassantPiece)
{
	if (!EnpassantPiece) return PRINTSTRING(FColor::Red, "EnpassantPiece Invalid in ChessBoard");

	AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "PlayerController Invalid in ChessBoard");

	// Set the enpassant target to the tile behind the pawn
	ChessBoardInfo.EnpassantPawn = EnpassantPiece->ChessPieceInfo;
	ChessBoardInfo.EnpassantTileIndex = ChessBoardInfo.EnpassantPawn.ChessPiecePositionIndex + ((ChessBoardInfo.EnpassantPawn.bIsWhite) ? -8 : 8);

	EnpassantPawn = EnpassantPiece;

	ChessPlayerController->OnPieceMoved.AddDynamic(this, &AChessBoard::DisableEnpassant);
}

void AChessBoard::DisableEnpassant(bool bIsWhite)
{
	if (bIsWhite != ChessBoardInfo.EnpassantPawn.bIsWhite)
	{
		ChessBoardInfo.EnpassantPawn = FChessPieceInfo();
		ChessBoardInfo.EnpassantTileIndex = -1;

		EnpassantPawn = nullptr;
		
		AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "PlayerController Invalid in ChessBoard");

		ChessPlayerController->OnPieceMoved.RemoveDynamic(this, &AChessBoard::DisableEnpassant);
	}
}

int32 AChessBoard::MoveGenerationTest(FChessBoardInfo BoardInfo, bool bIsWhiteTurn, int32 Depth)
{
	if (Depth == 0) return 1;

	int32 NumberOfPositions = 0;

	GenerateAllValidMoves(BoardInfo, bIsWhiteTurn);

	for (FChessTileInfo Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
			{
				for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
				{
					FChessBoardInfo SimulatedBoardInfo = BoardInfo;
					SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);
					NumberOfPositions += MoveGenerationTest(SimulatedBoardInfo, !bIsWhiteTurn, Depth - 1);
				}
			}
		}
	}

	return NumberOfPositions;
}