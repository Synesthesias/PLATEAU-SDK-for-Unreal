#pragma once

#include "Styling/SlateStyle.h"

/**
 * PLATEAU SDKにおけるエディタUIのスタイル定義
 */
class FPLATEAUEditorStyle : public FSlateStyleSet {

public:
    FPLATEAUEditorStyle();

    ~FPLATEAUEditorStyle();

private:
    FString InContent(
        const FString& RelativePath,
        const ANSICHAR* Extension);
};
