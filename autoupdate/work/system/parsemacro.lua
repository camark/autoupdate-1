-- UTM�궨������ű�

-- ȫ�ֱ���
intValue = 0			-- �Ⱥ��ұߵ���ֵ
strKeyName = ""			-- �Ⱥ���ߵĺ꣬�ַ���

-- ����������
-- ���أ�
--		����1�� 0--�ɹ� ����ʧ��
--		����2�� �ִ�
--		����3�� ����ֵ
function ParseLine(line)
	result = 1
	strKeyName = ""
	intValue = 0

	local lineType = CheckLineType(line)
	if lineType == 0 then
		local ret = ParseEqual(line)
		if ret == 0 then
			result = 0
		end
	else
		result = 1
	end
	
	if strKeyName == nil then strKeyName = "" end

	return result, strKeyName, intValue
end

-- �Ƿ�Ϊ�հ���
function IsSpaceLine(aline)
	local retLine = string.match(aline, "(%s*)")
	if retLine == aline then
		return true
	end

	return false
end

-- �Ƿ�Ϊ#�ſ�����
function IsNumbegSignLine(aline)
end

-- �ж�ǰ2���ַ�������
-- ����: 0--��������  ��0��--���ô���
function CheckLineType(aline)
	strBegin2 = string.match(aline, "%s*(..)")
	if strBegin2 == nil then			-- �������ַ�
		return 1
	elseif strBegin2 == "//" then		-- Ϊ"//"ע����
		return 2
	else
		return 0						-- ����
	end
end

-- ȡ=�����ߵ�ֵ
-- ����: 0--�ɹ�  ��0--ʧ��
function ParseEqual(aline)
	strKeyName, sTmpValue = string.match(aline, "%s*([%w_]*)%s*=%s*0[Xx]([%d%x]-),")

	-- __TW__DEBUG
	--print(strKeyName)
	--print(sTmpValue)

	if strKeyName == nil or strKeyName == "" or sTmpValue == nil or sTmpValue == "" then
		return 1
	end

	intValue = tonumber(sTmpValue, 16)

	-- __TW__DEBUG
	--print(intValue)

	return 0
end
