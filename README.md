＃＃ 系統要求
支持平台： -Windows 7或更高版本，32位或64位 

需要DirectX 9或OpenGL 4+支持的圖形驅動程序。

建議使用1 GB的可用磁盤空間來進行資產更新和映像。

建議使用1 GB的可用RAM，不過除非您向垃圾郵件重新啟動按鈕，否則遊戲的內存不應超過300 MB。

WindBot Ignite（AI）的前提條件：
-Windows：如果沒有，請安裝.NET Framework 4。 Windows 10附帶了該版本。

####################################################################
##鍵盤和鼠標快捷方式
ESC：如果不鍵入，則最小化窗口
F9：重新加載音頻
F11：切換全屏
F12：捕獲屏幕截圖
CTRL + O：打開其他設置窗口
R：如果不輸入則重新加載字體
CTRL + R：重新加載當前皮膚
CTRL + 1：切換至卡信息標籤
CTRL + 2：切換到對決日誌標籤
CTRL + 3：切換到聊天記錄標籤
CTRL + 4：切換到“設置”標籤
CTRL + 5：切換到存儲庫選項卡 *拖放支持文件和文本： *在主菜單或卡座編輯區域中放置一個“ydk”文件以加載該卡座 *在卡組編輯區域中放置卡密碼或卡名稱，以將該卡添加到卡組中 *在卡座編輯區域中放置一個ydke：//URL以加載該URL指定的卡座 *如果有效，請在主菜單或重播選擇菜單中放置一個yrpX文件以加載該重播 *如果有效，將Lua文件拖放到主菜單或拼圖選擇菜單中以加載該拼圖 *將文本放在文本框中以插入文本

####################################################################
###卡組編輯器： *鼠標右鍵：從卡組添加/刪除卡 *鼠標中鍵：將卡片的另一份副本添加到卡座或側面卡座

Shift +鼠標右鍵或按住鼠標左鍵，然後單擊鼠標右鍵：將卡片添加到側卡組 *除Shift +鼠標右鍵外，按住Shift鍵將忽略所有套牌構造規則
不輸入時：

CTRL + C：複製卡片組列表的ydke：//URL以進行共享
CTRL + SHIFT + C：複製純文本卡片組列表以進行共享
CTRL + V：從剪貼板導入“ ydke：//” URL卡片組列表

##卡組編輯器搜索功能 
string： 返回名稱或卡文本中帶有“string”的卡。 
`@string`： 返回屬於“string”字段的卡。
`$string`： 返回僅名稱中帶有“string”的卡，而忽略卡片文本。 
`string1||string2`： 返回名稱/文本中帶有“ string1”或“ string2”的卡。 
`!!string`： 否定查詢（不） *string1 * string2 替換任意數量的任何字符。
`string1*string2`：
例如：Eyes * Dragon：將返回卡片Blue-Eyes White Dragon，Red-Eyes B. Dragon，Galaxy-Eyes Photon Dragon等。
示例：@blue-eyes||$eyes of blue：返回屬於'Blue-Eyes'字段或名稱中包含'Eyes of Blue'的卡。

ATK，DEF，Level / Rank和Scale文本框支持搜索“？”。您還可以在搜索前添加比較修飾符<，<=，> =，>和=。

點一下卡名會打開LUA
點一下卡號將複制卡號

####################################################################
###決鬥： 
*按住A或按住鼠標左鍵：讓系統在每個時間停止。
*按住S或按住鼠標右鍵：讓系統跳過每個定時。 
*按住D：讓系統在可用的時間停止。

F1至F4：分別在GY，放逐的Extra卡組，Xyz物料中顯示卡。
F5至F8：分別在對手的GY，放逐的Extra卡組，Xyz的物料中顯示卡。

####################################################################
###皮膚： 
通過將子文件夾添加到“skin”中，可以進行編輯。對於每個文件夾，提供一個唯一的skin.xml文件，其中包含您想要的更改。 
在設置（CTRL + O）中切換皮膚。
有關受支持字段及其更改內容的說明，請參見“皮膚”中的自述文件。
*皮膚支持`textures'文件夾，以使這些項目更可定制。

####################################################################
*“Puzzle Mode”菜單可以讀取`puzzle`文件夾中的子目錄。

####################################################################
*“觀看重放”菜單讀取`replay`文件夾中的子目錄。

####################################################################
*禁止/限制列表現在保存在`lflists`目錄中：
*將`$ whitelist`添加到列表中將自動禁止該列表中未設置的所有條目
*可以從聊天/決鬥日誌中復製文本（CTRL + C）。