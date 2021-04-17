# PE2 -Text editor

【基本編輯功能與游標移動控制】
Cursor Up- 			游標上移
Cursor Down- 			游標下移
Cursor Left- 			游標左移
Cursor Right- 			游標右移
Home- 				移至行首
End- 				移至行尾
PgUp- 				上一頁
PgDn- 				下一頁
[Ctrl+Home] 			游標移動到到檔頭
[Ctrl+End]			游標移動到檔尾
[Ctrl+PgUp] 			游標移動到螢幕最上行
[Ctrl+Left] 			向左移動40個游標
[Ctrl+Right] 			向右移動40個游標
F5- 				刪除整行文字
F6- 				刪除游標後整行文字
Insert- 			切換Insert/Replace模式
Enter- 				換行
Del- 				刪除游標後單一文字
Backspace- 			刪除游標前單一文字

【文字標示與操作】
[Alt+B]- 			文字區塊方形選取
[Alt+C]- 			文字區整行選取有文字部分
[Alt+L]- 			文字區整行選取含空白部分
[Alt+U]- 			取消選取
[Alt+Z]- 			插入選取文字內容，保留選取區塊文字
[Alt+M]- 			插入選取文字內容，移除選取區塊文字
[Alt+O]- 			覆蓋選取文字內容，保留選取區塊文字
[Alt+D]- 			刪除選取文字

【尋找與取代】
/string- 			尋找文字	尋找指定文字[string]，例如 /hello，如果找到文字則由標停在該文字上，此時按[ctrl+Enter]鍵可以繼續尋找下一個相同文字。
c /str/new_str/-		取代文字	將文字[str]用[new_str]取代，例如c/hello/goodbye/，輸入命令後按[Ctrl+Enter]執行取代的動作。按[Shift+F5]執行取代的動作，但取代前會出現提示要求確認。

【檔案儲存與讀取】
e [[drive:][path]filename]-	新增或開啟檔案
				編輯一個檔案。有下列三種情況:
				1. 如果檔案已經在編輯緩衝區，則切換至該緩衝區繼續編輯。
				2. 如果檔案已經存在，則打開並讀取該檔案至新緩衝區，編輯該檔案。
				3. 如果檔案不存在，則開一個新的換衝區編輯。
				4. 如果只輸入 e 不接檔案名稱，則在已存在的緩衝區之間輪流切換編輯。
ESC- 				編輯模式/命令列模式
F2- 				儲存目前檔案
F3- 				儲存目前檔案並離開檔案
F4- 				離開目前檔案不存檔
n <filename>- 			儲存目前檔案並命名


