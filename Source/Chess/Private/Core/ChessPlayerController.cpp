// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Core/ChessPlayerController.h"

#include "Core/ChessGameMode.h"
#include "Board/ChessBoard.h"
#include "Board/ChessPiece.h"
#include "Board/ChessTile.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

#define PRINTSTRING(Colour, DebugMessage) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, Colour, DebugMessage);

AChessPlayerController::AChessPlayerController()
{
	bShowMouseCursor = true;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextAsset(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/+Chess/Input/IMC_Chess.IMC_Chess'"));
	if (InputMappingContextAsset.Succeeded()) InputMappingContext = InputMappingContextAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> SelectPieceActionAsset(TEXT("/Script/EnhancedInput.InputAction'/Game/+Chess/Input/Actions/IA_SelectPiece.IA_SelectPiece'"));
	if (SelectPieceActionAsset.Succeeded()) SelectPieceAction = SelectPieceActionAsset.Object;
}

void AChessPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameAndUI());
}

void AChessPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Select Piece
		EnhancedInputComponent->BindAction(SelectPieceAction, ETriggerEvent::Triggered, this, &AChessPlayerController::SelectPiece);
	}
}

void AChessPlayerController::SelectPiece()
{
	if (!bIsPlayerTurn) return PRINTSTRING(FColor::Green, "Is Not Player's Turn");

	AChessGameMode* ChessGameMode = Cast<AChessGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!ChessGameMode)
	{
		PRINTSTRING(FColor::Red, "Game Mode is invalid in PlayerController");
		//UE_LOG(LogTemp, Warning, TEXT("Game Mode is invalid in PlayerController"));
		return;
	}

	AChessBoard* ChessBoard = Cast<AChessBoard>(UGameplayStatics::GetActorOfClass(GetWorld(), AChessBoard::StaticClass()));
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is invalid in PlayerController");
		//UE_LOG(LogTemp, Warning, TEXT("ChessBoard is invalid in PlayerController"));
		return;
	}

	// Line trace to select a Tile
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Camera, false, HitResult); // trace channel is camera because chess piece ignores camera channel

	if (!HitResult.GetActor())
	{
		if (SelectedTile)
		{
			ChessBoard->HightlightValidMovesOnTile(false, SelectedTile->ChessTileInfo);
			SelectedTile = nullptr;
		}
		return;
	}

	AChessTile* HitTile = Cast<AChessTile>(HitResult.GetActor());
	if (!HitTile)
	{
		if (SelectedTile)
		{
			ChessBoard->HightlightValidMovesOnTile(false, SelectedTile->ChessTileInfo);
			SelectedTile = nullptr;
		}
		return;
	}

	if (SelectedTile) // if a tile is selected already, proceed to moving piece on that tile
	{
		if (!SelectedTile->ChessTileInfo.ChessPieceOnTile)
		{
			ChessBoard->HightlightValidMovesOnTile(false, SelectedTile->ChessTileInfo);
			SelectedTile = nullptr;
			return PRINTSTRING(FColor::Red, "ChessPieceOnTile is Invalid");
		}

		if (HitTile == SelectedTile)
		{
			ChessBoard->HightlightValidMovesOnTile(false, SelectedTile->ChessTileInfo);
			SelectedTile = nullptr;
			return PRINTSTRING(FColor::Red, "HitTile same as SelectedTile");
		}

		if (!HitTile->ChessTileInfo.bIsHighlighted)
		{
			ChessBoard->HightlightValidMovesOnTile(false, SelectedTile->ChessTileInfo);
			SelectedTile = nullptr;
			return PRINTSTRING(FColor::Red, "Tile not Highlighted");
		}

		if (HitTile->ChessTileInfo.ChessPieceOnTile) // if theres a piece on destination tile...
		{
			// and if the piece is a friendly piece...
			if (SelectedTile->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite == HitTile->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite)
			{
				return PRINTSTRING(FColor::Red, "Tile is Occupied with a friendly Piece");
			}
			else // and if the piece is not a friendly piece...
			{
				HitTile->ChessTileInfo.ChessPieceOnTile->CapturePiece(); // capture piece
			}
		}

		ChessBoard->HightlightValidMovesOnTile(false, SelectedTile->ChessTileInfo);

		SelectedTile->ChessTileInfo.ChessPieceOnTile->MovePiece(HitTile);
				
		HitTile->ChessTileInfo.ChessPieceOnTile = SelectedTile->ChessTileInfo.ChessPieceOnTile;
		HitTile->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.ChessPiecePositionIndex = HitTile->ChessTileInfo.ChessTilePositionIndex;

		SelectedTile->ChessTileInfo.ChessPieceOnTile = nullptr;
		SelectedTile = nullptr;

		OnPieceMoved.Broadcast(ChessGameMode->bIsWhiteTurn);

		ChessGameMode->SwitchTurn();
	}
	else // if no tile has been selected already, highlight valid moves for the piece on that tile
	{
		if (!HitTile->ChessTileInfo.ChessPieceOnTile)
		{
			PRINTSTRING(FColor::Red, "Tile is Empty");
			//UE_LOG(LogTemp, Warning, TEXT("Tile is Empty"));
			return;
		}

		if (ChessGameMode->bIsWhiteTurn != HitTile->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite)
		{
			PRINTSTRING(FColor::Red, "Not friendly Piece");
			//UE_LOG(LogTemp, Warning, TEXT("Not friendly Piece"));
			return;
		}

		if (ChessBoard->HightlightValidMovesOnTile(true, HitTile->ChessTileInfo))
			SelectedTile = HitTile;
	}
}