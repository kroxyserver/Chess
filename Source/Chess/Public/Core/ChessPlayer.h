// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ChessPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class CHESS_API AChessPlayer : public APawn
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Player", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Player", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Player", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent = nullptr;

public:
	AChessPlayer();

	FORCEINLINE USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent; }
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region FUNCTIONS

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "+Chess|Player")
	void SwitchPlayerView(bool bToWhite);

#pragma endregion

#pragma region VARIABLES

#pragma endregion
};