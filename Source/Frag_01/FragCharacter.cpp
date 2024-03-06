// Fill out your copyright notice in the Description page of Project Settings.


#include "FragCharacter.h"

// Sets default values
AFragCharacter::AFragCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AFragCharacter::ResetJumpCounter()
{
	JumpCurrentCount = 0;
	JumpCurrentCountPreJump = 0;
}

// Called when the game starts or when spawned
void AFragCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFragCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFragCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

