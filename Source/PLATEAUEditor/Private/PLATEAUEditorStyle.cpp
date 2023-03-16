// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUEditorStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Brushes/SlateBorderBrush.h"
#include "Brushes/SlateBoxBrush.h"
#include "Brushes/SlateImageBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Interfaces/IPluginManager.h"
#include "Math/Vector2D.h"
#include "Misc/Paths.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"

#define IMAGE_BRUSH(RelativePath, ...)                                         \
  FSlateImageBrush(                                                            \
      FPLATEAUEditorStyle::InContent(RelativePath, ".png"),                    \
      __VA_ARGS__)

#define BOX_BRUSH(RelativePath, ...)                                           \
  FSlateBoxBrush(                                                              \
      FPLATEAUEditorStyle::InContent(RelativePath, ".png"),                    \
      __VA_ARGS__)

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

FString FPLATEAUEditorStyle::InContent(
    const FString& RelativePath,
    const ANSICHAR* Extension) {
    static FString ContentDir = IPluginManager::Get()
        .FindPlugin(TEXT("PLATEAU-SDK-for-Unreal"))
        ->GetContentDir();
    return (ContentDir / RelativePath) + Extension;
}

FPLATEAUEditorStyle::FPLATEAUEditorStyle()
    : FSlateStyleSet("PLATEAUEditorStyle") {
    const FVector2D Icon30x30(30.0f, 30.0f);
    const FVector2D Icon180x141(180.0f, 141.0f);
    const FVector2D Icon256x256(256.0f, 256.0f);
    const FVector2D TabBackground(512.0f, 180.0f);
    
    Set("PLATEAUEditor.LogoImage", 
        new IMAGE_BRUSH("logo_for_unreal", FVector2D(270.0f, 67.5f)));
    Set("PLATEAUEditor.BuildingIconImage",
        new IMAGE_BRUSH("dark_icon_building", Icon30x30));
    Set("PLATEAUEditor.Section1IconImage",
        new IMAGE_BRUSH("dark_icon_1", Icon30x30));
    Set("PLATEAUEditor.Section2IconImage",
        new IMAGE_BRUSH("dark_icon_2", Icon30x30));
    Set("PLATEAUEditor.Section3IconImage",
        new IMAGE_BRUSH("dark_icon_3", Icon30x30));
    Set("PLATEAUEditor.TabImportIcon",
        new IMAGE_BRUSH("dark_icon_import", Icon180x141));
    Set("PLATEAUEditor.TabAdjustIcon",
        new IMAGE_BRUSH("dark_icon_adjust", Icon180x141));
    Set("PLATEAUEditor.TabExportIcon",
        new IMAGE_BRUSH("dark_icon_export", Icon180x141));
    Set("PLATEAUEditor.TabSelectBack",
        new IMAGE_BRUSH("round-button", Icon256x256));
    Set("PLATEAUEditor.TabBackground",
        new IMAGE_BRUSH("round-window-wide", TabBackground));
    Set("PLATEAUEditor.LogoBackground",
    new BOX_BRUSH("Old/Menu_Background", 0.0f, FLinearColor::FromSRGBColor(FColor(0xFF676767))));
    Set("PLATEAUEditor.LogoBorder",
        new BOX_BRUSH("Old/Menu_Background", 0.0f, FLinearColor::FromSRGBColor(FColor(0xFFD2D2D2))));

    Set(
        "PLATEAUEditor.Heading1",
        FTextBlockStyle()
        .SetColorAndOpacity(FSlateColor::UseForeground())
        .SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10)));

    Set(
        "PLATEAUEditor.Heading2",
        FTextBlockStyle()
        .SetColorAndOpacity(FSlateColor::UseForeground())
        .SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 12)));
    
    FSlateStyleRegistry::RegisterSlateStyle(*this);
}


FPLATEAUEditorStyle::~FPLATEAUEditorStyle() {
    FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef DEFAULT_FONT
