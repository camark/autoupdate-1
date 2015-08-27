-----------------------------------------------------------
-- �Զ����½ű�
-----------------------------------------------------------

-- ���õ���ģ��·��
local p = "./lua/"
local m_package_path = package.path
package.path = string.format("%s;%s?.lua;%s?/init.lua", m_package_path, p, p)
--package.cpath = string.format("%s;%s?.dll", m_package_path, p, p)
--C_PrintLog(package.cpath)

-----------------------------------------------------------
http=require("socket.http");
require("twutil")

-- ȫ�ֺ�
-- ��ģ�������ļ�
SYSTEM_CONFIG = "./autoupdate.ini"
-- ��exe�����ļ�
MAIN_CLIENT_CONFIG = "../Update_cfg.dat"


-----------------------------------------------------------
-- ȫ�ֱ���
-- ����url
g_dstUrl = ""
-- �ͻ���id
g_key = ""

-- ��ѯ�汾��ʱ��������λ��
g_checkVersionInterval = 900

-- ������Ҫ�����б� key:id value:{"sizeSourceFile"=sizeSourceFile, "path"=path}
g_downloadList = {}
g_needDownLoadCount = 0      -- ��Ҫ�����ļ�����
g_downloadedCount = 0        -- �������ļ���

g_savedPath = ".\\downloaded\\"
g_unrarPath = g_savedPath -- .. "unrar\\"
g_mediaPath = "../video/"
saveFilePath = ""            -- �����ļ������ص�ȫ·��
sizeSourceFile = 0           -- Դ�ļ���С
g_localFileName = ""         -- ���ص����ص��ļ���
-----------------------------------------------------------
-- ���ܺ���

-- ����http post����
-- �������:
--   url: ·��
--   body: ����
-- ����:
--   �ظ����ݣ��쳣ʱ���ؿ�
function HttpPost(url, body)
	--C_PrintLog("http post: " .. url .. "?" .. body)

	local request_body = body
	local response_body = {}

	local res, code, response_headers = http.request{
	  url = url,
	  method = "POST",
	  headers =
		{
			["Content-Type"] = "application/x-www-form-urlencoded";
			["Content-Length"] = #request_body;
		},
		source = ltn12.source.string(request_body),
		sink = ltn12.sink.table(response_body),
	}


	if 200 == code then
		if type(response_body) == "table" then
			--C_PrintLog(response_body[1])
			return response_body[1]
		else  -- ������˵�ַ���ܷ���ʱ���߸÷�֧
			C_PrintLog("[ERROR] http respone content isn't table value:" .. type(response_body))
			--C_PrintLog(response_body)
		end
	else
		C_PrintLog('http response code: ' .. code)
	end

	return ""
end

--
function Sleep(n)
   if n > 0 then os.execute("ping -n " .. tonumber(n + 1) .. " localhost > NUL") end
end

-----------------------------------------------------------
-- ���߼�
function BEGIN()
	-- ��ȡ����˵�ַ
	C_ReadConfig("port", "httpurl", MAIN_CLIENT_CONFIG)
	g_dstUrl = _Result
	if "" == g_dstUrl then
		C_PrintLog("[ERROR] server url nil, exit...")
		return
	end
	C_PrintLog("server url: " .. g_dstUrl)

	-- ��ȡ�ͻ���ID
	C_ReadConfig("port", "key", MAIN_CLIENT_CONFIG)
	g_key = _Result
	if "" == g_key then
		C_PrintLog("[ERROR] client id is nil, exit...")
		return
	end
	C_PrintLog("client id: " .. g_key)

	-- ��ȡ������ʱ����
	C_ReadConfig("system", "checkVersionInterval", SYSTEM_CONFIG)
	g_checkVersionInterval = tonumber(_Result)   -- ע�⣺��δ���ã���ֵΪnil
	if nil == g_checkVersionInterval or g_checkVersionInterval < 1 then
		g_checkVersionInterval = 900
	end
	C_PrintLog("check version interval: " .. g_checkVersionInterval)


	-- �ж�����Ŀ¼�Ƿ����
	if 0 ~= os.execute("cd " .. g_savedPath) then
		-- ��������Ŀ¼
		os.execute("md " .. g_savedPath)
	end

	-- �жϽ�ѹĿ¼�Ƿ����
	if 0 ~= os.execute("cd " .. g_unrarPath) then
		-- ��������Ŀ¼
		os.execute("md " .. g_unrarPath)
	end

	stWaitForCheckVersion()
end

-- �ȴ���һ�β�ѯ�汾
function stWaitForCheckVersion()
	C_PrintLog("stWaitForCheckVersion:")

	-- �����״̬����ʾ��Ҫ���ò���
	g_downloadList = {}
	g_needDownLoadCount = 0      -- ��Ҫ�����ļ�����
	g_downloadedCount = 0        -- �������ļ���

	C_StartTimer(g_checkVersionInterval)
	C_SetNextState("_EvtTimeOut", -1, "stCheckVersion")
end

-- ������������ȫ���ļ��б������һ���ļ���Ϣ
function AddFile(line)
	local path, sizeSourceFile = string.match(line, "(.+),(%d+)")
	if nil== path or ""==path then
		C_PrintLog("file info error: " .. line)
		return
	end

	local fileInfo = {["path"] = path, ["sizeSourceFile"] = sizeSourceFile}
	g_downloadList[g_needDownLoadCount] = fileInfo
	g_needDownLoadCount = g_needDownLoadCount + 1
end

-- ��ʾ�������б�
function ShowDownloadLiest()
	C_PrintLog("--------- download list ---------")
	local i = 0
	while i < g_needDownLoadCount do
		C_PrintLog(g_downloadList[i]["path"] .. "," .. g_downloadList[i]["sizeSourceFile"])
		i = i + 1
	end
end


-- ��������: �����������ļ��б���Ϣ
function ParseNeedUpdateFileList(str)
	local beginPos = 1
	local commaPos = string.find(str, ";", beginPos)

	local line = ""
	while(commaPos ~= nil) do
		--C_PrintLog(commaPos)
		line = string.sub(str, beginPos, commaPos)

		-- ģ��
		--line = "http://demo.zksr.cn/upload/priceTag/ccc250.mp4,255929468"

		AddFile(line)
		--C_PrintLog(line)
		beginPos = commaPos + 1
		commaPos = string.find(str, ";", beginPos)
	end

	-- ���һ���ļ�����Ψһ��һ���ļ�
	line = string.sub(str, beginPos)

	-- ģ��
	--line = "http://demo.zksr.cn/upload/priceTag/bbb.wmv,32069384"

	--C_PrintLog(line)
	AddFile(line)

	ShowDownloadLiest()
end

-- ����Ƿ��и���
function stCheckVersion()
	C_PrintLog("stCheckVersion:")

	-- ����http���������
	local body = "method=checkVersion&key=" .. g_key
	local resp = HttpPost(g_dstUrl, body)

	if nil == resp then
		C_PrintLog("response content is nil")
		-- ��Ϊ��ѯʧ��
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stWaitForCheckVersion")

		return
	end
	
	C_PrintLog("check version resp: " .. resp)

	-- �������
	local status, filelist = string.match(resp, "(%d+)|(.*)")
	if nil == status or nil == filelist then
		C_PrintLog('error response: ' .. resp)
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stWaitForCheckVersion")
		return
	end

	-- �ж�״̬
	local nStatus = tonumber(status)
	if 0 == nStatus then       -- û�и���
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stWaitForCheckVersion")
	elseif 1 == nStatus then  -- �и��£����ҿ�����������
		-- ���������б�
		ParseNeedUpdateFileList(filelist)
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stDownload")
	elseif 2 == nStatus then  -- �и��£���������������
		-- ���������б�
		ParseNeedUpdateFileList(filelist)
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stCheckForDownload")
	else
		C_PrintLog("Unknown status " .. status)
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stWaitForCheckVersion")
	end
end

-- ��ѯ�Ƿ�ɸ���
function stCheckForDownload()
	C_PrintLog("stCheckForDownload:")

	local body = "method=checkUpdate&key=" .. g_key
	local resp = HttpPost(g_dstUrl, body)

	if nil == resp then
		C_PrintLog("response content is nil")
		-- ��Ϊ��ѯʧ��
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stCheckForDownload")

		return
	end

	--
	C_PrintLog("check download resp: " .. resp)

	-- �������
	local status = tonumber(resp)
	if 1 == status then  -- ��������
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stDownload")
		return
	else                 -- ����������
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stCheckForDownload")
	end
end

-- �����ļ�
function stDownload()
	C_PrintLog("stDownload:")

	--
	stIsDownloadOver()
end

-- �Ƿ��������
function stIsDownloadOver()
	C_PrintLog("stIsDownloadOver:")

	if g_downloadedCount < g_needDownLoadCount then
		stDownloadOne()
	else -- �������
		stNotifyServerOver()
	end
end

-- ����һ�������ļ�
function HandleDownoadedFile(fileFullPath, filename)
	--
	--filename = "test.rar"
	--fileFullPath = ".\\test.rar"

	local ext = GetExtNameofFile(filename)
	if "" == ext then
		C_PrintLog("no ext name " .. filename)
		return
	end

	
	if "rar" == ext then   -- ѹ���ļ�
		os.execute("rar x -o+ " .. fileFullPath .. " " .. g_unrarPath)
		-- ɾ��ѹ���ļ�
		os.remove(fileFullPath)
	else
		-- �����ļ���ʱȫ����Ϊý���ļ���ֱ�ӿ�����ý��Ŀ¼  TODO: ��Ҫȷ������rar֮�ⶼ��ý���ļ�
		ret = C_CopyFile(fileFullPath, g_mediaPath .. filename) -- ��c������п�����Ŀ���������ļ���
		-- ɾ��ý���ļ�
		os.remove(fileFullPath)
	end
end

-- ������һ���ļ�
function stDownloadOne()
	C_PrintLog("stDownloadOne:")

	-- ��װ�����ļ���
	local path = g_downloadList[g_downloadedCount]["path"]
	sizeSourceFile = g_downloadList[g_downloadedCount]["sizeSourceFile"]
	C_PrintLog("download file: " .. path .. " sizeSourceFile: " .. sizeSourceFile)

	g_localFileName = ParseFileNameInPath(path)
	saveFilePath = g_savedPath .. g_localFileName
	C_PrintLog("save file: " .. saveFilePath)

	local ret = C_HttpDownload(path, saveFilePath)	-- �첽����http�ļ�
	if 0 ~= ret then  -- ���ͬ������ʧ�ܣ���Ҫ����������Ƿ���Ҫ��������
		C_PrintLog("[ERROR] download fail! " .. path)
		C_StartTimer(1)
		C_SetNextState("_EvtTimeOut", -1, "stDownloadOne")
		return
	end

	-- �ж��Ƿ��������
	C_StartTimer(1)
	C_SetNextState("_EvtTimeOut", -1, "stIsDownloadOK")
end

-- �ж��Ƿ��������
function stIsDownloadOK()
	C_PrintLog("stIsDownloadOK:")

	if 1 ~= C_IsHttpDownloadOK() then -- ����δ���
		-- ��������
		local body = "method=heartbeat&key=" .. g_key
		local resp = HttpPost(g_dstUrl, body)
		C_PrintLog("heart beat resp " .. resp)

		-- ���60�룬���ж��Ƿ��������
		C_StartTimer(60)
		C_SetNextState("_EvtTimeOut", -1, "stIsDownloadOK")
		return
	end

	-- �������
	-- ͨ�������ļ���С�ж��Ƿ����سɹ�
	local savedFileSize = GetFileSize(saveFilePath)
	C_PrintLog("download ok, savedFileSize = " .. savedFileSize)
	if tonumber(savedFileSize) ~= tonumber(sizeSourceFile) then                     -- �����ļ���Դ�ļ���С��һ�£���Ҫ��������
		C_PrintLog("[ERROR] download file sizeSourceFile error! re download")
		C_StartTimer(5)
		C_SetNextState("_EvtTimeOut", -1, "stDownloadOne")
		return
	end

	-- ���غ�Ĵ���
	HandleDownoadedFile(saveFilePath, g_localFileName)

	-- ������������
	g_downloadedCount = g_downloadedCount + 1

	-- ���ñ��ظ��±�־ ���ļ��ж�����
	local pfUpdateFlagFile = io.open("updateflag.dat", "w")
	if nil == pfUpdateFlagFile then
		C_PrintLog("[ERROR] Write update flag failed")
	end
	pfUpdateFlagFile:write("[info]\nneedupdate=1\n")
	pfUpdateFlagFile:close()

	C_StartTimer(1)
	C_SetNextState("_EvtTimeOut", -1, "stIsDownloadOver")
end

-- ֪ͨ������������
function stNotifyServerOver()
	C_PrintLog("stNotifyServerOver:")

	-- ����֪ͨ�ӿ�
	local body = "method=updateFinish&key=" .. g_key
	local resp = HttpPost(g_dstUrl, body)

	if nil == resp or resp ~= "1" then
		C_PrintLog("response content is nil")
		-- ��Ϊʧ��
		C_StartTimer(5)
		C_SetNextState("_EvtTimeOut", -1, "stNotifyServerOver")

		return
	end

	C_PrintLog("notify result: " .. resp)

	-- ��ʼ��һ�θ��²�ѯ�ȴ�
	C_StartTimer(1)
	C_SetNextState("_EvtTimeOut", -1, "stWaitForCheckVersion")
end


function stEnd()
	C_PrintLog("END:")

	C_Exit()
end


BEGIN()
