# UnitTestの追加方法

## 新規テストBP追加
### １．Blueprint Class作成メニューから親となるクラスにPLATEAUBaseTestを選択します。

<img src='UnitTestImages/image.png' width='50%'>
<br>
<br>

### ２．出力するログファイル名を設定
String型の変数を作成しデフォルト値として、このテストが完了した時に出力するログファイル名を設定します。

<img src='UnitTestImages/image2.png' width='50%'>
<br>
<br>

### ３．Event Start Testを実装
Event Graphを開きEvent Start Testノードから必要な処理を実装します。<br>
* 既に出力されているログファイルを削除したい場合はDelete Test Resultを利用します。<br>
* InstancedCityModelを利用するテストの場合は親クラスのStart Test Eventを利用します。<br>
これにより、InstancedCityModelがシーンに存在する状態でテストを実行できます。<br>
既にInstancedCityModelがシーンに存在する場合は新たに生成しません。
* InstancedCityModelを利用しないテストの場合は４章で実装するRun Testをそのまま利用します。<br>



<img src='UnitTestImages/image3.png' width='50%'>
<br>
<br>


### ４．Run Testを実装
親クラスのRun TestをOverrideしてRun Testを実装します。親クラスのRun Testを呼ぶノードが自動生成されますが削除します。<br>
テストが成功した場合はSucceeded Testノードを最終ノードとして実行します。<br>
テストが失敗した場合はFailed Testノードを最終ノードとして実行します。

<img src='UnitTestImages/image4.png' width='50%'>
<br>
<img src='UnitTestImages/image5.png' width='50%'>
<br>
<br>

### ５．TestLevelを編集後、Session Frontendを確認
TestLevelに作成したテストBPをドラッグアンドドロップでレベルに配置します。<br>
Tools/Session Frontendを開きProject/Functional Tests以下を確認すると該当のテストが追加されています。<br>
表示されていなかった場合は上部メニューのRefresh Testsを実行します。

<img src='UnitTestImages/image6.png' width='50%'><br>
<img src='UnitTestImages/image7.png' height='700px'>
