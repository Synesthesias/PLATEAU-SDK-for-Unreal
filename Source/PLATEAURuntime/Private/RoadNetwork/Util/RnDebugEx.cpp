#include "RoadNetwork/Util/RnDebugEx.h"

FVector FRnDebugEx::ToVec3(const FVector2D& Self, bool bShowXZ) {
    return bShowXZ ? FVector(Self.X, 0.0f, Self.Y) : FVector(Self.X, Self.Y, 0.0f);
}

FLinearColor FRnDebugEx::GetDebugColor(int32 I, int32 Num, float A) {
    const float H = 1.0f * (I % Num) / Num;
    FLinearColor Color = FLinearColor::MakeFromHSV8(H * 255, 255, 255);
    Color.A = A;
    return Color;
}

void FRnDebugEx::DrawLine(const FVector& Start, const FVector& End, const FLinearColor& Color, float Duration, float Thickness) {
    DrawDebugLine(GWorld, Start, End, Color.ToFColor(true), false, Duration, 0, Thickness);
}

void FRnDebugEx::DrawArrow(const FVector& Start, const FVector& End, float ArrowSize, const FVector& ArrowUp, const FLinearColor& BodyColor, const FLinearColor& ArrowColor, float Duration, float Thickness) {
    DrawLine(Start, End, BodyColor, Duration, Thickness);

    if (ArrowSize > 0.0f) {
        const FVector Direction = (End - Start).GetSafeNormal();
        const FVector Right = FVector::CrossProduct(Direction, ArrowUp).GetSafeNormal();
        const FVector Up = FVector::CrossProduct(Right, Direction).GetSafeNormal();

        const FVector Arrow1 = End - Direction * ArrowSize + Right * ArrowSize;
        const FVector Arrow2 = End - Direction * ArrowSize - Right * ArrowSize;

        DrawLine(Arrow1, End, ArrowColor, Duration, Thickness);
        DrawLine(Arrow2, End, ArrowColor, Duration, Thickness);
    }
}
void FRnDebugEx::DrawArrows(const TArray<FVector>& Vertices, bool bIsLoop, float ArrowSize, const FVector& ArrowUp, const FLinearColor& Color, const FLinearColor& ArrowColor, float Duration, float Thickness) {
    for (int32 i = 0; i < Vertices.Num() - 1; ++i) {
        DrawArrow(Vertices[i], Vertices[i + 1], ArrowSize, ArrowUp, Color, ArrowColor, Duration, Thickness);
    }

    if (bIsLoop && Vertices.Num() > 1) {
        DrawArrow(Vertices.Last(), Vertices[0], ArrowSize, ArrowUp, Color, ArrowColor, Duration, Thickness);
    }
}

void FRnDebugEx::DrawLines(const TArray<FVector>& Vertices, bool bIsLoop, const FLinearColor& Color, float Duration, float Thickness) {
    for (int32 i = 0; i < Vertices.Num() - 1; ++i) {
        DrawLine(Vertices[i], Vertices[i + 1], Color, Duration, Thickness);
    }

    if (bIsLoop && Vertices.Num() > 1) {
        DrawLine(Vertices.Last(), Vertices[0], Color, Duration, Thickness);
    }
}

void FRnDebugEx::DrawSphere(const FVector& Center, float Radius, const FLinearColor& Color, float Duration) {
    DrawDebugSphere(GWorld, Center, Radius, 16, Color.ToFColor(true), false, Duration);
}

void FRnDebugEx::DrawRegularPolygon(const FVector& Center, float Radius, int32 Sides, const FVector& Up, const FLinearColor& Color, float Duration) {
    const float AngleStep = 2.0f * PI / Sides;
    const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Up);

    TArray<FVector> Points;
    for (int32 i = 0; i < Sides; ++i) {
        const float Angle = AngleStep * i;
        const FVector Point = Center + Rotation.RotateVector(FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f));
        Points.Add(Point);
    }

    DrawLines(Points, true, Color, Duration);
}

void FRnDebugEx::DrawDashedLine(const FVector& Start, const FVector& End, const FLinearColor& Color, float LineLength, float SpaceLength, float Duration) {
    const FVector Direction = (End - Start).GetSafeNormal();
    const float TotalLength = FVector::Distance(Start, End);
    const float SegmentLength = LineLength + SpaceLength;
    const int32 NumSegments = FMath::CeilToInt(TotalLength / SegmentLength);

    for (int32 i = 0; i < NumSegments; ++i) {
        const float StartDist = i * SegmentLength;
        const float EndDist = FMath::Min(StartDist + LineLength, TotalLength);

        if (EndDist > StartDist) {
            const FVector LineStart = Start + Direction * StartDist;
            const FVector LineEnd = Start + Direction * EndDist;
            DrawLine(LineStart, LineEnd, Color, Duration);
        }
    }
}

void FRnDebugEx::Log(const FString& Message) {
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
}

void FRnDebugEx::LogWarning(const FString& Message) {
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

void FRnDebugEx::LogError(const FString& Message) {
    UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
}

void FRnDebugEx::DrawDashedArrow(const FVector& Start, const FVector& End, const FLinearColor& Color, float LineLength, float SpaceLength, float Duration, float ArrowSize, const FVector& ArrowUp) {
    const FVector Direction = (End - Start).GetSafeNormal();
    const float TotalLength = FVector::Distance(Start, End);
    const float SegmentLength = LineLength + SpaceLength;
    const int32 NumSegments = FMath::CeilToInt(TotalLength / SegmentLength);

    for (int32 i = 0; i < NumSegments; ++i) {
        const float StartDist = i * SegmentLength;
        const float EndDist = FMath::Min(StartDist + LineLength, TotalLength);

        if (EndDist > StartDist) {
            const FVector LineStart = Start + Direction * StartDist;
            const FVector LineEnd = Start + Direction * EndDist;
            DrawArrow(LineStart, LineEnd, ArrowSize, ArrowUp, Color, Color, Duration);
        }
    }
}

void FRnDebugEx::DrawDashedLines(const TArray<FVector>& Vertices, bool bIsLoop, const FLinearColor& Color, float LineLength, float SpaceLength, float Duration) {
    for (int32 i = 0; i < Vertices.Num() - 1; ++i) {
        DrawDashedLine(Vertices[i], Vertices[i + 1], Color, LineLength, SpaceLength, Duration);
    }

    if (bIsLoop && Vertices.Num() > 1) {
        DrawDashedLine(Vertices.Last(), Vertices[0], Color, LineLength, SpaceLength, Duration);
    }
}

void FRnDebugEx::DrawDashedArrows(const TArray<FVector>& Vertices, bool bIsLoop, const FLinearColor& Color, float LineLength, float SpaceLength, float Duration) {
    for (int32 i = 0; i < Vertices.Num() - 1; ++i) {
        DrawDashedArrow(Vertices[i], Vertices[i + 1], Color, LineLength, SpaceLength, Duration);
    }

    if (bIsLoop && Vertices.Num() > 1) {
        DrawDashedArrow(Vertices.Last(), Vertices[0], Color, LineLength, SpaceLength, Duration);
    }
}

void FRnDebugEx::DrawRay(const FVector& Start, const FVector& Direction, const FLinearColor& Color, float Duration) {
    const float RayLength = 1000.0f; // Default ray length
    DrawLine(Start, Start + Direction * RayLength, Color, Duration);
}

void FRnDebugEx::DrawString(const FString& Text, const FVector& Location, const FLinearColor& Color, float Duration,
    int32 FontSize)
{
    DrawDebugString(GWorld, Location, Text, nullptr, Color.ToFColor(true), Duration, false, FontSize);
}
