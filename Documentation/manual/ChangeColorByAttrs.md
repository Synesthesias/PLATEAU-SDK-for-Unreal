# 属性情報によって色分けする
![](../resources/manual/changeColorByAttrs/AttributeColorSample.png)

このサンプルでは、土地計画決定情報に関する属性情報を読み取り、それに応じてランタイムで色を変えます。  

属性情報を読み取り、色を変えるBlueprintは次の場所にあります:  
```(PLATEAU SDKのサンプルディレクトリ)/AttributesColorSample/AttributesColorSampleLogic```

このBlueprintでは、都市の各ゲームオブジェクトに付与されているコンポーネントである`PLATEAUCityObjectGroup`から情報を読み取ります。  
同コンポーネントから`CityObject`を取得し、そこから属性情報である`AttributesMap`にアクセスできます。  
詳しくはBlueprintを参照してください。