local banned_html = [[
<html lang = "ru">
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<title>BANNED IP ADDRESS</title>
	</head>
	<body>
		<div align = "center">
			<h1>Ваш IP Address был забанен</h1>
		</div>
	</body>
</html>
]]

local notfound_html = [[
<html lang = "ru">
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<title>404 - Не найдено</title>
	</head>
	<body>
		<div align = "center">
			<h1>404 - Страница не найдена</h1>
		</div>
	</body>
</html>
]]

function OnGetRequest(client,request,ipaddress)
	if ipaddress == "192.168.1.1" then
		SendHTMLCode(false,client,banned_html)
	else
		if request == "" or request == "index" then
			SendHTMLPage(false,client,"html\\index.html")
		elseif request == "downloadfile_step1" then
			SendHTMLPage(false,client,"html\\success_download.html")
		elseif request == "favicon.ico" then
			SendImage(client,"html\\data\\icon1.ico")
		elseif request == "image1" then
			SendImage(client,"html\\data\\bitmap1.bmp")
		elseif request == "image2" then
			SendImage(client,"html\\data\\image2.png")
		elseif request == "downloadfile_step2" then
			SendFile(client,"html\\data\\HexEditor.exe","HexEditor_v1.0.0.1.exe")
		else
			SendHTMLCode(false,client,notfound_html)
		end
	end
end

function OnPostRequest(client,request,thread,ipaddress)
	if request == "uploadfile" then
		-- SAVE FILE USING LUA from file html\data\temp\temp_%thread%.tmp
		SendHTMLPage(false,client,"html\\success_upload.html")
	elseif request == "imput1" then
		-- SEND HTML CODE USING LUA from file html\data\temp\temp_%thread%.tmp
	else
		SendHTMLPage(false,client,"html\\index.html")
	end
end