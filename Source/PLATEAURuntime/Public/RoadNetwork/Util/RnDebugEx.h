#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "RnDebugEx.generated.h"

USTRUCT()
struct PLATEAURUNTIME_API FRnDrawOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool bVisible = true;

    UPROPERTY(EditAnywhere)
    FLinearColor Color = FLinearColor::White;
};

struct PLATEAURUNTIME_API FRnDebugEx
{
public:
    static FVector ToVec3(const FVector2D& Self, bool bShowXZ);
    static FLinearColor GetDebugColor(int32 I, int32 Num = 8, float A = 1.0f);
    static void DrawLine(const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f);
    static void DrawArrow(const FVector& Start, const FVector& End, float ArrowSize = 0.5f, const FVector& ArrowUp = FVector::UpVector, const FLinearColor& BodyColor = FLinearColor::White, const FLinearColor& ArrowColor = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f);
    static void DrawArrows(const TArray<FVector>& Vertices, bool bIsLoop = false, float ArrowSize = 0.5f, const FVector& ArrowUp = FVector::UpVector, const FLinearColor& Color = FLinearColor::White, const FLinearColor& ArrowColor = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f);
    static void DrawLines(const TArray<FVector>& Vertices, bool bIsLoop = false, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f);
    static void DrawSphere(const FVector& Center, float Radius, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f);
    static void DrawRegularPolygon(const FVector& Center, float Radius, int32 Sides = 5, const FVector& Up = FVector::UpVector, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f);
    static void DrawDashedLine(const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::White, float LineLength = 1.0f, float SpaceLength = 0.2f, float Duration = 0.0f);

    static void DrawDashedArrow(const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::White, float LineLength = 1.0f, float SpaceLength = 0.2f, float Duration = 0.0f, float ArrowSize = 0.5f, const FVector& ArrowUp = FVector::UpVector);
    static void DrawDashedLines(const TArray<FVector>& Vertices, bool bIsLoop = false, const FLinearColor& Color = FLinearColor::White, float LineLength = 3.0f, float SpaceLength = 1.0f, float Duration = 0.0f);
    static void DrawDashedArrows(const TArray<FVector>& Vertices, bool bIsLoop = false, const FLinearColor& Color = FLinearColor::White, float LineLength = 3.0f, float SpaceLength = 1.0f, float Duration = 0.0f);
    static void DrawRay(const FVector& Start, const FVector& Direction, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f);
    static void DrawString(const FString& Text, const FVector& Location,  const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f, float FontScale = 1.f);
};