-- ͨ�ù��ܺ���

local http=require("socket.http");

-- ������ȫ·���е��ļ���
function ParseFileNameInPath(path)
	local beginPos = 1
	local keyPos = string.find(path, "[/\\]", beginPos)
	while(keyPos ~= nil) do
		beginPos = keyPos + 1
		keyPos = string.find(path, "[/\\]", beginPos)
	end
	savedPath = string.sub(path, beginPos)
	return savedPath
end

-- ����http�ļ�
-- ����
--		0:�ɹ�  ����ʧ��
function DownLoadHttpFile(url, savedPath)
	local output = io.open(savedPath, "wb");
	if nil == output then
		print("open saved file faild: " .. savedPath)
		return 1
	end

	print("begin downloading at " .. os.date())
	local response, status, respHeaders, statusLine = http.request(url)
	--print(response)
	--print(status)
	--print(respHeaders)
	--print(statusLine)
	output:write(response)
	print("end downloading at " .. os.date())

	output:close()

	return 0
end

-- ��ȡ�ļ���С
function GetFileSize(filePath)
	local hFile = io.open(filePath, "rb")
	if nil == hFile then
		return 0
	end

    --local currentPos = hFile:seek() -- ��ȡ��ǰλ��
    local size = hFile:seek("end") -- ��ȡ�ļ���С
    --file:seek("set", currentPos)
	hFile:close()
    return size
end

-- ��ȡ�ļ���չ��
function GetExtNameofFile(filename)
	local ext = string.match(filename, ".+%.(.+)")
	return string.lower(ext or "")
end
