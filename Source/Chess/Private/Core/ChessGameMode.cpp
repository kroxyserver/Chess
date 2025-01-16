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
	bIsWhiteTurn(true)
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
		break;
	case EChessGameModeType::Player_VS_Player:
		ChessPlayerController->bIsPlayerTurn = true;
		break;
	default:
		break;
	}
}

void AChessGameMode::SwitchTurn()
{
	bIsWhiteTurn = !bIsWhiteTurn;
	
	if (!ChessBoard) return PRINTSTRING(FColor::Red, "ChessBoard is Invalid in GameMode");

	ChessBoard->GenerateAllValidMoves(bIsWhiteTurn);

	switch (ChessGameModeType)
	{
	case EChessGameModeType::Player_VS_AI:
		if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "ChessPlayerController is Invalid in GameMode");
		ChessPlayerController->bIsPlayerTurn = !ChessPlayerController->bIsPlayerTurn;
		
		//TODO : MORE CODE NEEDED HERE FOR AI FUNCTIONALITY


		break;
	case EChessGameModeType::Player_VS_Player:
		if (!ChessPlayer) return PRINTSTRING(FColor::Red, "ChessPlayer is Invalid in GameMode");
		ChessPlayer->SwitchPlayerView(bIsWhiteTurn);
		break;
	default:
		break;
	}
}