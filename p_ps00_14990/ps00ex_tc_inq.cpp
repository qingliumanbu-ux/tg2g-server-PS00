/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2010
Author:		yanl
Version:    1.0
Date:		2018-01-03
Description:作业计划电文履历查询－电文号查询
**************************************************/

/*<remark>=========================================================
/// <summary>
/// 电文号查询
/// <para>获取电文号或电文分类配置代码</para>
/// <para>查询电文号及电文名称； </para>
/// </summary>
/// <param name="EP01_CODE"> 电文分类配置代码 。</param>
/// <param name="TC_CODE"> 电文号 。</param>
/// <returns>无</returns>
===========================================================</remark>*/
#include "stdafx.h"

// Service 入口
BM2F_ENTERACE(ps00ex_tc_inq)

int f_ps00ex_tc_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	int RowCount = 0;
	int v_cnt = 0;

	/* 业务变量 */
	CString		v_ep01_code = " ";  /* 电文分类配置代码 */
	CString		v_tc_code = " ";  /* 电文号 */

	/* 实体类定义 */


	// 数据库SQL操作字符串
	CString  sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_conn(conn);
	CDbCommand cmd_inq(conn);

	try
	{
		//-----------------------------------------------------------------------
		//获得输入参数
		v_ep01_code = bcls_rec->Tables[0].Rows[0]["EP01_CODE"];
		v_tc_code = bcls_rec->Tables[0].Rows[0]["TC_CODE"];

		Log::Trace("", __FUNCTION__, "=== = [{0}][{1}]", v_ep01_code, v_tc_code);
		
		//设置返回列
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "TC_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "TC_NAME");
		bcls_ret->Tables[0].Columns["TC_NO"].set_Caption("电文号");
		bcls_ret->Tables[0].Columns["TC_NAME"].set_Caption("电文名称");

		//获取工序类型代码
		if (v_tc_code.Trim() != "")
		{
			sqlstr = "SELECT TC_NO,TC_NAME "
				"	    FROM TEXT1_RES  "
				"	   WHERE TC_NO LIKE @v_tc_code||'%'   "
				"	     AND CULTURE = 'zh_Hans'" 
				;
		}
		else
		{
			sqlstr = "SELECT B.TC_NO, B.TC_NAME FROM TEP0002 A, TEXT1_RES B "
				"	   WHERE A.CODE_CLASS = @v_ep01_code "
				"		 AND A.CODE = B.TC_NO "
				"		 AND B.CULTURE = 'zh_Hans'";
		}

		Log::Trace("", __FUNCTION__, "==== sqlstr = [{0}]", sqlstr);
		cmd_conn.SetCommandText(sqlstr);
		cmd_conn.Parameters.Set("v_ep01_code", v_ep01_code);
		cmd_conn.Parameters.Set("v_tc_code", v_tc_code);
		cmd_conn.ExecuteReader();

		RowCount = 0;
		while (cmd_conn.Read())
		{
			// 将结果放入返回块			
			CDataRow& row = bcls_ret->Tables[0].Rows.Add();
			row["TC_NO"] = cmd_conn.GetString(1);
			row["TC_NAME"] = cmd_conn.GetString(2);
			
		}//while
		cmd_conn.Close();

		{
			CFormattable arguments[] = { RowCount };// 定义参数列表的数组
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
	cmd_conn.Close();

	return doFlag;
}
