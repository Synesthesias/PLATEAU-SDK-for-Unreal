// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUMeshCodeGizmo.h"

#include "SceneManagement.h"

#include <plateau/dataset/mesh_code.h>
#include <plateau/geometry/geo_reference.h>

#include "CanvasTypes.h"
#include "Algo/AnyOf.h"
#include "Engine/Font.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialRenderProxy.h"

namespace {
    bool IsAboveLevel4(const plateau::dataset::MeshCode& MeshCode) {
        return MeshCode.getLevel() >= 4;
    }

    int GetNumAreaColumnByMeshCode(const plateau::dataset::MeshCode& MeshCode) {
        return IsAboveLevel4(MeshCode) ? 2 : 4;
    }

    int GetNumAreaRowByMeshCode(const plateau::dataset::MeshCode& MeshCode) {
        return IsAboveLevel4(MeshCode) ? 2 : 4;
    }

    const TArray<FString> SuffixMeshIds = {
        TEXT("11"), TEXT("12"), TEXT("21"), TEXT("22"),
        TEXT("13"), TEXT("14"), TEXT("23"), TEXT("24"),
        TEXT("31"), TEXT("32"), TEXT("41"), TEXT("42"),
        TEXT("33"), TEXT("34"), TEXT("43"), TEXT("44")};

    // 選択色
    constexpr FColor SelectedColor = FColor(255, 204, 153);
    constexpr FColor UnselectedColor = FColor(0, 0, 0, 0);

    int GetRowIndex(const double InMinX, const double InMaxX, const int InNumGrid, const double InValue) {
        const double GridSize = (InMaxX - InMinX) / InNumGrid;
        for (int i = 0; i < InNumGrid; i++) {
            if (InValue <= InMinX + GridSize * (i + 1)) {
                return i;
            }
        }
        return InNumGrid - 1;
    }

    int GetColumnIndex(const double InMinY, const double InMaxY, const int InNumGrid, const double InValue) {
        const double GridSize = (InMaxY - InMinY) / InNumGrid;
        for (int i = 0; i < InNumGrid; i++) {
            if (InMaxY - GridSize * (i + 1) <= InValue) {
                return i;
            }
        }
        return InNumGrid - 1;
    }
}

FPLATEAUMeshCodeGizmo::FPLATEAUMeshCodeGizmo() : MeshCode(), Width(0), Height(0), MinX(-500), MinY(-500), MaxX(500), MaxY(500), LineThickness(1.0f) {
    AreaSelectedMaterial = DuplicateObject(GEngine->ConstraintLimitMaterialPrismatic, nullptr);
    AreaSelectedMaterial.Get()->SetScalarParameterValue(FName("Desaturation"), 1.0f);
    AreaSelectedMaterial.Get()->SetVectorParameterValue(FName("Color"), SelectedColor);
    AreaUnSelectedMaterial = DuplicateObject(GEngine->ConstraintLimitMaterialPrismatic, nullptr);
    AreaUnSelectedMaterial.Get()->SetScalarParameterValue(FName("Desaturation"), 1.0f);
    AreaUnSelectedMaterial.Get()->SetVectorParameterValue(FName("Color"), UnselectedColor);
}

bool FPLATEAUMeshCodeGizmo::IsSelectable() const {
    if (MeshCode.getLevel() <= 2) return false;
    return true;
}

void FPLATEAUMeshCodeGizmo::ResetSelectedArea() {
    for (int i = 0; i < bSelectedArray.Num(); i++) {
        bSelectedArray[i] = false;
    }
}

void FPLATEAUMeshCodeGizmo::DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const {
    const FBox Box(FVector(MinX, MinY, 0), FVector(MaxX, MaxY, 0));
    const auto Color = FColor(10, 10, 130);
    const int NumAreaColumn = GetNumAreaColumnByMeshCode(MeshCode);
    const int NumAreaRow = GetNumAreaRowByMeshCode(MeshCode);

    // エリア枠線
    DrawWireBox(PDI, Box, Color, SDPG_World, LineThickness, 0, true);

    if (!bShowLevel5Mesh)
        return;
   
    // 格子状のライン
    for (int i = 1; i <= NumAreaColumn -1; ++i) {
        const auto X1 = (Box.Min.X * i + Box.Max.X * (NumAreaColumn - i)) / NumAreaColumn;
        const auto Py1 = Box.Min.Y;
        const auto Qy1 = Box.Max.Y;
        constexpr auto Z = 0.0;
        const FVector P1(X1, Py1, Z);
        const FVector Q1(X1, Qy1, Z);
        PDI->DrawLine(P1, Q1, Color, SDPG_World, 1, 0, true);
    }

    for (int i = 1; i <= NumAreaRow - 1; ++i) {
        const auto Y2 = (Box.Min.Y * i + Box.Max.Y * (NumAreaRow - i)) / NumAreaRow;
        const auto Px2 = Box.Min.X;
        const auto Qx2 = Box.Max.X;
        constexpr auto Z = 0.0;
        const FVector P2(Px2, Y2, Z);
        const FVector Q2(Qx2, Y2, Z);
        PDI->DrawLine(P2, Q2, Color, SDPG_World, 1, 0, true);
    }

    // エリア塗りつぶし
    const auto CellWidth = (Box.Max.X - Box.Min.X) / NumAreaRow;
    const auto CellHalfWidth = (Box.Max.X - Box.Min.X) / (NumAreaRow * 2);
    const auto CellHeight = (Box.Max.Y - Box.Min.Y) / NumAreaColumn;
    const auto CellHalfHeight = (Box.Max.Y - Box.Min.Y) / (NumAreaColumn * 2);

    for (int Col = 0; Col < NumAreaColumn; Col++) {
        for (int Row = 0; Row < NumAreaRow; Row++) {
            if (bSelectedArray[Row + Col * NumAreaColumn]) {
                FMatrix ObjectToWorld = FMatrix::Identity;
                ObjectToWorld.SetAxis(0, FVector(CellHalfWidth, 0, 0));
                ObjectToWorld.SetAxis(1, FVector(0, CellHalfHeight, 0));

                //Level4は1Gridずらす
                int AdjustedRow = IsAboveLevel4(MeshCode) ? Row + 1 : Row;
                int AdjustedCol = IsAboveLevel4(MeshCode) ? Col + 1 : Col;
                ObjectToWorld.SetOrigin(FVector((Box.Min.X + Box.Max.X) / 2 - CellHalfWidth - CellWidth + CellWidth * AdjustedRow,
                                                (Box.Min.Y + Box.Max.Y) / 2 + CellHalfHeight + CellHeight - CellHeight * AdjustedCol, 0));
                DrawPlane10x10(PDI, ObjectToWorld, 1.0f, FVector2D::Zero(), FVector2D::One(), AreaSelectedMaterial->GetRenderProxy(), SDPG_Foreground);
            }
        }
    }
}

void FPLATEAUMeshCodeGizmo::DrawRegionMeshID(const FViewport& InViewport, const FSceneView& View, FCanvas& Canvas, const FString& RegionMeshID,
                                             double CameraDistance, int IconCount) const {
    constexpr auto NearOffset = 8000;
    constexpr auto FarOffset = 100000;

    const auto CenterX = MinX + (MaxX - MinX) / 2;
    const auto Coef = 4 < IconCount ? 1.52 : 1.28;
    const auto CenterY = MinY + (MaxY - MinY) / 2 * Coef;

    const auto dpi = Canvas.GetDPIScale();
    const auto ViewPlane = View.Project(FVector(CenterX, CenterY, 0));

    const auto HalfX = InViewport.GetSizeXY().X / 2 / dpi;
    const auto HalfY = InViewport.GetSizeXY().Y / 2 / dpi;

    const auto XPos = HalfX + (HalfX * ViewPlane.X);
    const auto YPos = HalfY + (HalfY * -ViewPlane.Y);

    const UFont* ViewFont = GEngine->GetLargeFont();

    const auto HalfWidth = ViewFont->GetStringSize(*RegionMeshID) / 2;
    const auto HalfHeight = ViewFont->GetStringHeightSize(*RegionMeshID) / 2;

    const auto Color = FColor::Blue;

    if (ViewPlane.W > 0.f) {
        if (CameraDistance <= NearOffset) {
            Canvas.DrawShadowedText(XPos - HalfWidth, YPos - HalfHeight, FText::FromString(RegionMeshID), ViewFont, Color);
        }
    }
}

FVector2D FPLATEAUMeshCodeGizmo::GetMin() const {
    return FVector2D(MinX, MinY);
}

FVector2D FPLATEAUMeshCodeGizmo::GetMax() const {
    return FVector2D(MaxX, MaxY);
}

FVector2D FPLATEAUMeshCodeGizmo::GetSize() const {
    return FVector2D(Width, Height);
}

plateau::dataset::MeshCode FPLATEAUMeshCodeGizmo::GetMeshCode() const {
    return MeshCode;
}

FString FPLATEAUMeshCodeGizmo::GetRegionMeshID() const {
    return MeshCodeString;
}

TArray<bool> FPLATEAUMeshCodeGizmo::GetbSelectedArray() const {
    return bSelectedArray;
}

bool FPLATEAUMeshCodeGizmo::bSelectedArea() const {
    return Algo::AnyOf(bSelectedArray, [](const bool bSelected) {
        return bSelected;
    });
}

void FPLATEAUMeshCodeGizmo::SetbSelectedArray(const TArray<bool>& InbSelectedArray) {
    bSelectedArray = InbSelectedArray;
}

void FPLATEAUMeshCodeGizmo::Init(const plateau::dataset::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference) {
    const auto Extent = InMeshCode.getExtent();
    const auto RawMin = InGeoReference.project(Extent.min);
    const auto RawMax = InGeoReference.project(Extent.max);
    MeshCode = InMeshCode;
    MeshCodeString = UTF8_TO_TCHAR(InMeshCode.get().c_str());
    Width = MaxX - MinX;
    Height = MaxY - MinY;
    MinX = FGenericPlatformMath::Min(RawMin.x, RawMax.x);
    MinY = FGenericPlatformMath::Min(RawMin.y, RawMax.y);
    MaxX = FGenericPlatformMath::Max(RawMin.x, RawMax.x);
    MaxY = FGenericPlatformMath::Max(RawMin.y, RawMax.y);
    LineThickness = 2.0f;

    const int NumAreaColumn = GetNumAreaColumnByMeshCode(MeshCode);
    const int NumAreaRow = GetNumAreaRowByMeshCode(MeshCode);

    bSelectedArray.Reset();
    for (int i = 0; i < NumAreaRow * NumAreaColumn; i++) {
        bSelectedArray.Emplace(false);
    }
}

void FPLATEAUMeshCodeGizmo::ToggleSelectArea(const double X, const double Y) {

    if (!IsSelectable()) 
        return;

    if ((MinX <= X && X <= MaxX && MinY <= Y && Y <= MaxY) == false) {
        return;
    }

    int NumAreaColumn = GetNumAreaColumnByMeshCode(MeshCode);
    int NumAreaRow = GetNumAreaRowByMeshCode(MeshCode);
    const auto RowIndex = GetRowIndex(MinX, MaxX, NumAreaRow, X);
    const auto ColumnIndex = GetColumnIndex(MinY, MaxY, NumAreaColumn, Y);
    bSelectedArray[RowIndex + ColumnIndex * NumAreaColumn] = !bSelectedArray[RowIndex + ColumnIndex * NumAreaColumn];
}

void FPLATEAUMeshCodeGizmo::SetSelectArea(const FVector2d InMin, const FVector2d InMax, const bool bSelect) {

    if (!IsSelectable()) 
        return;

    const int NumAreaColumn = GetNumAreaColumnByMeshCode(MeshCode);
    const int NumAreaRow = GetNumAreaRowByMeshCode(MeshCode);   
    const auto CellWidth = (MaxX - MinX) / NumAreaRow;
    const auto CellHeight = (MaxY - MinY) / NumAreaColumn;
    for (int Col = 0; Col < NumAreaColumn; Col++) {
        for (int Row = 0; Row < NumAreaRow; Row++) {
            const auto RectMinX = MinX + CellWidth * Row;
            const auto RectMaxX = MinX + CellWidth * (Row + 1);
            const auto RectMinY = MaxY - CellHeight * (Col + 1);
            const auto RectMaxY = MaxY - CellHeight * Col;
            if (RectMaxX < InMin.X || RectMinX > InMax.X || RectMaxY < InMin.Y || RectMinY > InMax.Y) {
                continue;
            }
            bSelectedArray[Row + Col * NumAreaColumn] = bSelect;
        }
    }
}

void FPLATEAUMeshCodeGizmo::SetSelectArea(const double X, const double Y, const bool bSelect) {

    if (!IsSelectable()) 
        return;
    
    if ((MinX <= X && X <= MaxX && MinY <= Y && Y <= MaxY) == false) {
        return;
    }

    const int NumAreaColumn = GetNumAreaColumnByMeshCode(MeshCode);
    const int NumAreaRow = GetNumAreaRowByMeshCode(MeshCode);
    const auto RowIndex = GetRowIndex(MinX, MaxX, NumAreaRow, X);
    const auto ColumnIndex = GetColumnIndex(MinY, MaxY, NumAreaColumn, Y);
    bSelectedArray[RowIndex + ColumnIndex * NumAreaColumn] = bSelect;
}

TArray<FString> FPLATEAUMeshCodeGizmo::GetSelectedMeshIds() {

    const int NumAreaColumn = GetNumAreaColumnByMeshCode(MeshCode);
    const int NumAreaRow = GetNumAreaRowByMeshCode(MeshCode);

    TArray<FString> MeshIdArray;
    for (int Col = 0; Col < NumAreaColumn; Col++) {
        for (int Row = 0; Row < NumAreaRow; Row++) {
            if (bSelectedArray[Row + Col * NumAreaColumn]) {
                MeshIdArray.Emplace(FString::Format(TEXT("{0}{1}"), {MeshCodeString, SuffixMeshIds[Row + Col * NumAreaColumn]}));
            }
        }
    }

    return MeshIdArray;
}

void FPLATEAUMeshCodeGizmo::SetShowLevel5Mesh(const bool bValue) {
    bShowLevel5Mesh = bValue;
}
