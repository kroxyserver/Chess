// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Core/ChessGameMode.h"

#include "Board/ChessBoard.h"
#include "Core/ChessGameInstance.h"
#include "Core/ChessPlayer.h"
#include "Core/ChessPlayerController.h"

#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#define PRINTSTRING(Colour, DebugMessage) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, Colour, DebugMessage);

AChessGameMode::AChessGameMode() :
	ChessBoardClass(AChessBoard::StaticClass()),
	ChessGameModeType(EChessGameModeType::Player_VS_Player),
	ChessGameState(EChessGameState::Ongoing),
	bIsWhiteTurn(false)
{
	DefaultPawnClass = AChessPlayer::StaticClass();
	PlayerControllerClass = AChessPlayerController::StaticClass();
}

void AChessGameMode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AChessGameMode::BeginPlay()
{
	Super::BeginPlay();

	UChessGameInstance* ChessGameInstance = Cast<UChessGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!ChessGameInstance) return PRINTSTRING(FColor::Red, "ChessGameInstance is Invalid in GameMode");

	ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "ChessPlayerController is Invalid in GameMode");

	ChessPlayer= Cast<AChessPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!ChessPlayer) return PRINTSTRING(FColor::Red, "ChessPlayer is Invalid in GameMode");



	// Spawn Chessboard at Chess Board Spawn Point
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(
		GetWorld(),
		ATargetPoint::StaticClass(),
		"ChessBoardSpawnPoint",
		OutActors
	);

	if (!OutActors.IsValidIndex(0)) return;
	{
		ChessBoard = GetWorld()->SpawnActor<AChessBoard>(ChessBoardClass, OutActors[0]->GetActorTransform());
		if (!ChessBoard) return PRINTSTRING(FColor::Red, "ChessBoard is Invalid in GameMode");
	}



	ChessGameModeType = ChessGameInstance->ChessGameModeType;

	switch (ChessGameModeType)
	{
	case EChessGameModeType::Player_VS_AI:
		ChessPlayerController->bIsPlayerTurn = UKismetMathLibrary::RandomBool();

		if (!ChessPlayerController->bIsPlayerTurn)
		{
			ChessBoard->BlackOpeningMove = UKismetMathLibrary::RandomInteger(ChessBoard->BlackOpeningMoves.Num() - 1);
		}
		else
		{
			ChessBoard->WhiteOpeningMove = UKismetMathLibrary::RandomInteger(ChessBoard->WhiteOpeningMoves.Num() - 1);
		}

		// Switch Initial Player view to Black if playing against AI
		ChessPlayer->SwitchPlayerView(!ChessPlayerController->bIsPlayerTurn);

		break;
	case EChessGameModeType::Player_VS_Player:
		ChessPlayerController->bIsPlayerTurn = true;
		break;
	default:
		break;
	}

	SwitchTurn(); // Start Playing
}

void AChessGameMode::SwitchTurn()
{
	if (!ChessBoard) return PRINTSTRING(FColor::Red, "ChessBoard is Invalid in GameMode");
	if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "ChessPlayerController is Invalid in GameMode");
	if (!ChessPlayer) return PRINTSTRING(FColor::Red, "ChessPlayer is Invalid in GameMode");
	
	bIsWhiteTurn = !bIsWhiteTurn;
	
	OnSwitchTurn.Broadcast();

	ChessBoard->GenerateAllValidMoves(ChessBoard->ChessBoardInfo, bIsWhiteTurn);

	// Check if GameOver
	ChessGameState = ChessBoard->IsGameOver(ChessBoard->ChessBoardInfo, bIsWhiteTurn);
		
	if (ChessGameState != EChessGameState::Ongoing) return ChessPlayerController->SpawnGameOverUI(ChessGameState, bIsWhiteTurn);

	switch (ChessGameModeType)
	{
	case EChessGameModeType::Player_VS_AI:
		ChessPlayerController->bIsPlayerTurn = !ChessPlayerController->bIsPlayerTurn;
				
		if (!ChessPlayerController->bIsPlayerTurn) // if AI's turn, then move AI piece
		{
			ChessPlayerController->DisableInput(ChessPlayerController);

			if ((bIsWhiteTurn)
				? (ChessBoard->WhiteOpeningIndex < ChessBoard->WhiteOpeningMoves[ChessBoard->WhiteOpeningMove].Num())	// if AI is White and White Opening Moves are available...
				: (ChessBoard->BlackOpeningIndex < ChessBoard->BlackOpeningMoves[ChessBoard->BlackOpeningMove].Num()))	// if AI is Black and Black Opening Moves are available...
			{
				// Do an Opening Move
				if (bIsWhiteTurn)
				{
					FChessMove Move = ChessBoard->WhiteOpeningMoves[ChessBoard->WhiteOpeningMove][ChessBoard->WhiteOpeningIndex];
					ChessBoard->MakeMove(ChessBoard->ChessTiles[Move.FromIndex], ChessBoard->ChessTiles[Move.ToIndex], true);
					ChessBoard->WhiteOpeningIndex++;
				}
				else
				{
					FChessMove Move = ChessBoard->BlackOpeningMoves[ChessBoard->BlackOpeningMove][ChessBoard->BlackOpeningIndex];
					ChessBoard->MakeMove(ChessBoard->ChessTiles[Move.FromIndex], ChessBoard->ChessTiles[Move.ToIndex], true);
					ChessBoard->BlackOpeningIndex++;
				}

				FTimerHandle TH_MakeMoveDelay;
				GetWorldTimerManager().SetTimer(
					TH_MakeMoveDelay,
					[this]()
					{
						ChessPlayerController->EnableInput(ChessPlayerController);
						ChessPlayerController->OnPieceMoved.Broadcast(bIsWhiteTurn);
						SwitchTurn();
					},
					.1f,
					false
				);
			}
			else
			{
				Async(EAsyncExecution::ThreadPool, [this]()
				{
					FChessMove BestMove = ChessBoard->CalculateBestAIMove(ChessBoard->ChessBoardInfo, bIsWhiteTurn, ChessBoard->MaxSearchDepthForAIMove);

					Async(EAsyncExecution::TaskGraphMainThread, [this, BestMove]()
					{
						if (BestMove.FromIndex == -1 || BestMove.ToIndex == -1)	return PRINTSTRING(FColor::Red, "CHECKMATE, Player Wins / Invalid Best Move for AI");

						ChessBoard->MakeMove(ChessBoard->ChessTiles[BestMove.FromIndex], ChessBoard->ChessTiles[BestMove.ToIndex], true);

						FTimerHandle TH_MakeMoveDelay;
						GetWorldTimerManager().SetTimer(
							TH_MakeMoveDelay,
							[this]()
							{
								PRINTSTRING(FColor::Green, "enable Input");
								ChessPlayerController->EnableInput(ChessPlayerController);
								ChessPlayerController->OnPieceMoved.Broadcast(bIsWhiteTurn);
								SwitchTurn();
							},
							.1f,
							false
						);
					});
				});
			}
		}
		
		break;
	case EChessGameModeType::Player_VS_Player:
		ChessPlayer->SwitchPlayerView(bIsWhiteTurn);
		break;
	default:
		break;
	}
}