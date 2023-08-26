// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUEditorStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "Math/Vector2D.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/StyleColors.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(FPLATEAUEditorStyle::InContent(RelativePath, ".png"), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(FPLATEAUEditorStyle::InContent(RelativePath, ".png"), __VA_ARGS__)
// #define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
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
    const FVector2D Icon16x16(16.0f, 16.0f);
    const FVector2D Icon30x30(30.0f, 30.0f);
    const FVector2D Icon180x141(180.0f, 141.0f);
    const FVector2D Icon256x256(256.0f, 256.0f);
    const FVector2D TabBackground(512.0f, 180.0f);

    Set("PLATEAUEditor.LogoImage", new IMAGE_BRUSH("logo_for_unreal", FVector2D(270.0f, 67.5f)));
    Set("PLATEAUEditor.BuildingIconImage", new IMAGE_BRUSH("dark_icon_building", Icon30x30));
    Set("PLATEAUEditor.Section1IconImage", new IMAGE_BRUSH("dark_icon_1", Icon30x30));
    Set("PLATEAUEditor.Section2IconImage", new IMAGE_BRUSH("dark_icon_2", Icon30x30));
    Set("PLATEAUEditor.Section3IconImage", new IMAGE_BRUSH("dark_icon_3", Icon30x30));
    Set("PLATEAUEditor.TabImportIcon", new IMAGE_BRUSH("dark_icon_import", Icon180x141));
    Set("PLATEAUEditor.TabAdjustIcon", new IMAGE_BRUSH("dark_icon_adjust", Icon180x141));
    Set("PLATEAUEditor.TabExportIcon", new IMAGE_BRUSH("dark_icon_export", Icon180x141));
    Set("PLATEAUEditor.TabSelectBack", new IMAGE_BRUSH("round-button", Icon256x256));
    Set("PLATEAUEditor.TabBackground", new IMAGE_BRUSH("round-window-wide", TabBackground));
    Set("PLATEAUEditor.Lod01", new IMAGE_BRUSH("lod01", Icon30x30));
    Set("PLATEAUEditor.Lod02", new IMAGE_BRUSH("lod02", Icon30x30));
    Set("PLATEAUEditor.Lod03", new IMAGE_BRUSH("lod03", Icon30x30));
    Set("PLATEAUEditor.Lod04", new IMAGE_BRUSH("lod04", Icon30x30));
    
    FLinearColor TransBackground = FLinearColor::FromSRGBColor(FColor::Black);
    TransBackground.A = 0.6f;
    Set("PLATEAUEditor.FloatingBorder", new FSlateRoundedBoxBrush(TransBackground, 8.f));
    
	const FTextBlockStyle NormalText = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
    Set("PLATEAUEditor.Bold.14", FTextBlockStyle(NormalText).SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 14)));
    Set("PLATEAUEditor.Bold.13", FTextBlockStyle(NormalText).SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 13)));

    Button = FButtonStyle().
        SetNormal(BOX_BRUSH("Common/Button", FVector2D(32,32), 8.0f/32.0f)).
        SetHovered(BOX_BRUSH("Common/Button_Hovered", FVector2D(32,32), 8.0f/32.0f)).
        SetPressed(BOX_BRUSH("Common/Button_Pressed", FVector2D(32,32), 8.0f/32.0f)).
        SetNormalPadding(FMargin(2, 2, 2, 2)).SetPressedPadding(FMargin(2, 3, 2, 1));
    
    struct ButtonColor {
        FName Name;
        FLinearColor Normal;
        FLinearColor Hovered;
        FLinearColor Pressed;

        ButtonColor(const FName& InName, const FLinearColor& Color) : Name(InName) {
            Normal = Color * 0.8f;
            Normal.A = Color.A;
            Hovered = Color * 1.0f;
            Hovered.A = Color.A;
            Pressed = Color * 0.6f;
            Pressed.A = Color.A;
        }
    };
    TArray<ButtonColor> FlatButtons;
    FlatButtons.Add(ButtonColor("PLATEAUEditor.FlatButton.Gray", FLinearColor(0.1, 0.1, 0.1)));

    for (const ButtonColor& Entry : FlatButtons) {
        Set(Entry.Name, FButtonStyle(Button)
            .SetNormal(BOX_BRUSH("FlatButton", 2.0f / 8.0f, Entry.Normal))
            .SetHovered(BOX_BRUSH("FlatButton", 2.0f / 8.0f, Entry.Hovered))
            .SetPressed(BOX_BRUSH("FlatButton", 2.0f / 8.0f, Entry.Pressed)));
    }

    const FCheckBoxStyle CheckboxLookingToggleButtonStyle = FCheckBoxStyle().SetCheckBoxType(ESlateCheckBoxType::ToggleButton).
        SetUncheckedImage(IMAGE_BRUSH("PlateauCheckBox", Icon16x16)).
        SetUncheckedHoveredImage(IMAGE_BRUSH("PlateauCheckBox", Icon16x16, FLinearColor( 0.5f, 0.5f, 0.5f ))).
        SetUncheckedPressedImage(IMAGE_BRUSH("PlateauCheckBox", Icon16x16, FLinearColor( 0.2f, 0.2f, 0.2f ))).
        SetCheckedImage(IMAGE_BRUSH("PlateauCheck", Icon16x16)).
        SetCheckedHoveredImage(IMAGE_BRUSH("PlateauCheck", Icon16x16, FLinearColor( 0.5f, 0.5f, 0.5f ))).
        SetCheckedPressedImage(IMAGE_BRUSH("PlateauCheck", Icon16x16, FLinearColor( 0.2f, 0.2f, 0.2f ))).
        SetPadding(1.0f);
    Set("PlateauCheckboxLookToggleButtonCheckbox", CheckboxLookingToggleButtonStyle);
    
    FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FPLATEAUEditorStyle::~FPLATEAUEditorStyle() {
    FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

#undef IMAGE_BRUSH
#undef DEFAULT_FONT
