/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2010
Author:		yanl
Version:    1.0
Date:		2017-01-05
Description:作业计划电文履历查询
**************************************************/
#include "stdafx.h"

//业务头

//外部函数声明
int f_edsetcustominfo(EIClass * bcls_rec, EIClass * bcls_ret);

// service入口
BM2F_ENTERACE(ps00ex_inq)

int f_ps00ex_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag = 0;
	int		rowCount = 0;

	/* 业务变量 */
	CString		function_id = "PS00EX_INQ";	/* 功能号 */
	CString		t_plan_no = " ";			/* 计划号 */
	CString		t_in_mat_no = " ";			/* 材料号 */
	CString		t_tc_no = " ";				/* 电文号 */
	CString		t_process_code = " ";		/* 电文处理代码 */
	CString		v_time_start = "";			/* 记录时间起 */
	CString		v_time_end = "";			/* 记录时间止 */
	CString		t_key_in_mat_no = " ";		/* 材料号关键字段 */
	CString		t_key_plan_no = " ";		/* 计划号关键字段 */
	
	/* 实体类定义 */
	// 数据库SQL操作字符串
	CString  sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_mat_inq(conn);

	try
	{
		//-----------------------------------------------------------------------
		//获得输入参数
		t_tc_no			= bcls_rec->Tables[0].Rows[0]["TC_NO"];
		t_plan_no		= bcls_rec->Tables[0].Rows[0]["PLAN_NO"];
		t_in_mat_no		= bcls_rec->Tables[0].Rows[0]["IN_MAT_NO"].ToString().Trim();
		t_process_code	= bcls_rec->Tables[0].Rows[0]["PROCESS_CODE"].ToString().Trim();
		v_time_start	= bcls_rec->Tables[0].Rows[0]["TIME_START"].ToString().Trim();
		v_time_end		= bcls_rec->Tables[0].Rows[0]["TIME_END"].ToString().Trim();

		Log::Trace("", __FUNCTION__, "=== =  [{0}][{1}][{2}][{3}][{4}]"
			, t_tc_no, t_plan_no, t_in_mat_no, v_time_start, v_time_end);

		bcls_rec->Tables[0].Columns.Add(DT_STRING, "FUNCTION_ID");
		bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"] = function_id;
		
		Log::Trace("", __FUNCTION__, "===function_id =  [{0}]", function_id);
		if (bcls_ret->Tables.get_Count() < 1)
		{
			//在bcls_ret 中增加一个块，放查询结果
			bcls_ret->Tables.Add();
		}
		f_edsetcustominfo(bcls_rec, bcls_ret);

		CDecimal i_cnt = 0;
		sqlstr = "SELECT A.TC_ITEM_NAME  "
				" FROM TEXT2 A "
				"WHERE A.TC_NO = @t_tc_no "
				"  AND A.KEY_FLAG = '1' "
				" ORDER BY A.TC_ITEM_SEQ_NO " ; 
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("t_tc_no", t_tc_no);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			i_cnt = i_cnt + 1;
			if (cmd_inq.GetString(1).Trim() == "plan_no")
			{
				//计划号对应的关键字段名(小写)
				t_key_plan_no = "KEY_VALUE_" + i_cnt.ToString();
			}
			if (cmd_inq.GetString(1).Trim() == "in_mat_no")
			{
				//计划号对应的关键字段名
				t_key_in_mat_no = "KEY_VALUE_" + i_cnt.ToString();;
			}
		}
		cmd_inq.Close();

		//查询计划明细信息
		sqlstr = "SELECT TC_ID,TC_NO,TC_TIME,PROCESS_CODE,PROCESS_MESSAGE,TC_TYPE "
				"	, KEY_VALUE_1, KEY_VALUE_2, KEY_VALUE_3, KEY_VALUE_4, KEY_VALUE_5 "
				"FROM TEXT3 	"
				"  WHERE  TC_NO = @t_tc_no ";
		if (t_process_code.Trim() != "")
		{
			sqlstr = sqlstr + " AND PROCESS_CODE = @t_process_code  ";
		}
		if (v_time_start.Trim() != "")
		{
			sqlstr = sqlstr + " AND TC_TIME >= @v_time_start ";
		}
		if (v_time_end.Trim() != "")
		{
			v_time_end = v_time_end.SubstringNE(0, 8) + "2400";
			sqlstr = sqlstr + "AND TC_TIME <= @v_time_end ";
		}

		if (t_plan_no.Trim() != "" && t_key_plan_no.Trim() != "")
		{
			sqlstr = sqlstr + " AND " + t_key_plan_no + " LIKE @t_plan_no || '%'   ";
		}
		if (t_in_mat_no.Trim() != "" && t_key_in_mat_no.Trim() !="")
		{
			sqlstr = sqlstr + " AND " + t_key_in_mat_no + " LIKE @t_in_mat_no || '%'   ";
		}
		sqlstr = sqlstr + " ORDER BY TC_TIME DESC    ";

		cmd_mat_inq.SetCommandText(sqlstr);
		cmd_mat_inq.Parameters.Set("t_tc_no", t_tc_no);
		cmd_mat_inq.Parameters.Set("t_plan_no", t_plan_no.Trim());
		cmd_mat_inq.Parameters.Set("t_in_mat_no", t_in_mat_no.Trim());
		cmd_mat_inq.Parameters.Set("t_process_code", t_process_code);
		cmd_mat_inq.ExecuteReader();

		Log::Trace("", __FUNCTION__, "=== sqlstr=  [{0}]", sqlstr);
		//循环从游标中取数据，压回前台
		while (cmd_mat_inq.Read())
		{
			//将结果放入返回块
			CDataRow& row = bcls_ret->Tables[0].Rows.Add(); //新增一行
			rowCount = bcls_ret->Tables[0].Rows.get_Count();

			row["TC_ID"] = cmd_mat_inq.GetString(1);
			row["TC_NO"] = cmd_mat_inq.GetString(2);
			row["TC_TIME"] = cmd_mat_inq.GetString(3);
			row["PROCESS_CODE"] = cmd_mat_inq.GetDecimal(4);
			row["PROCESS_MESSAGE"] = cmd_mat_inq.GetString(5);
			row["TC_TYPE"] = cmd_mat_inq.GetString(6);
			row["KEY_VALUE_1"] = cmd_mat_inq.GetString(7);
			row["KEY_VALUE_2"] = cmd_mat_inq.GetString(8);
			row["KEY_VALUE_3"] = cmd_mat_inq.GetString(9);
			row["KEY_VALUE_4"] = cmd_mat_inq.GetString(10);
			row["KEY_VALUE_5"] = cmd_mat_inq.GetString(11);

		} 
		cmd_mat_inq.Close();

		{
			CFormattable arguments[] = { bcls_ret->Tables[0].Rows.get_Count() };// 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004")/*查询到[{0}]条记录。*/, arguments, 1);//格式化字符串
		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	cmd_inq.Close();

	return doFlag;
}
