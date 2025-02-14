// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

/**
 * モデル変換で、あとでマテリアルを復元したい場合に使います。indexがマテリアルID、値がマテリアルです。
 */
class PLATEAURUNTIME_API FPLATEAUCachedMaterialArray
{
public:
    /**
     * 追加してそのインデックスを返します。
     * ただし、すでに同じマテリアルが配列中にある場合は、追加せずにそのインデックスを返します。
     */
    int Add(TObjectPtr<UMaterialInterface> Mat)
    {
        if (Mat == nullptr)
            return -1;

        // 同じマテリアルが既に存在するかチェック
        for (int i=0; i<Materials.Num(); i++)
        {
            const auto& Material = Materials[i];
            if (Material == Mat)
                return i;
        }

        // 存在しない場合は追加
        Materials.Add(Mat);
        return Materials.Num() - 1;
    }

    TObjectPtr<UMaterialInterface> Get(int Index) const
    {
        return Materials[Index];
    }

    int Num() const
    {
        return Materials.Num();
    }

    /**
     * キャッシュされたマテリアルをすべてクリアします。
     */
    void Clear()
    {
        Materials.Empty();
    }

    /**
     * 配列からMatを探してそのインデックスを返します。なければ-1を返します。
     */
    int IndexOf(TObjectPtr<UMaterialInterface> Mat)
    {
        if (Mat == nullptr)
            return -1;

        // 配列から同じマテリアルを探す
        for (int i = 0; i < Materials.Num(); i++)
        {
            const auto& Material = Materials[i];
            if (Material == Mat)
                return i;
        }

        return -1;
    }

    void SetDefaultMaterial(UMaterialInterface* Material) {
        DefaultMaterial = Material;
    }

    UMaterialInterface* GetDefaultMaterial() const {
        return DefaultMaterial;
    }

private:
    TArray<TObjectPtr<UMaterialInterface>> Materials;

    UMaterialInterface* DefaultMaterial;
};
