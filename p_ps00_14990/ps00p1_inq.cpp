/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		yanl
Version:    1.0
Date:		2018-08-02
Description:收池参数过滤_查询
**************************************************/

#include "stdafx.h"


// service入口
BM2F_ENTERACE(ps00p1_inq)
int f_ps00p1_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag = 0;
	int		newbk = 1;
	
	/* 实体类定义 */
	CModel tps00p1("TPS00P1");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	try
	{
		//-----------------------------------------------------------------------
		//获得输入参数
		tps00p1["USER_ID"] = s.userid;
		tps00p1["FORM_NAME"] = s.formname;
		tps00p1["TYPE_CODE"] = bcls_rec->Tables[0].Rows[0]["TYPE_CODE"];
		tps00p1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];

		Log::Trace("", __FUNCTION__, "=== =  [{0}][{1}][{2}][{3}][{4}]"
			, tps00p1["FORM_NAME"].ToString(), tps00p1["PLAN_BACKLOG_CODE"].ToString(), tps00p1["USER_ID"].ToString(), tps00p1["TYPE_CODE"].ToString());

		sqlstr = "SELECT  *  FROM TPS00P1 "
			"  WHERE PLAN_BACKLOG_CODE = @tps00p1.PLAN_BACKLOG_CODE "
			"  AND FORM_NAME = @tps00p1.FORM_NAME  ";
		if (tps00p1["TYPE_CODE"].ToString() == "1")
		{
			//定制，按工号
			sqlstr = sqlstr +
				"  AND USER_ID = @tps00p1.USER_ID  ";
		}
		else if (tps00p1["TYPE_CODE"].ToString() == "0")
		{
			//公用，工号为空
			sqlstr = sqlstr +
				"  AND USER_ID = ' '  ";
		}
		else 
		{
			//查询公用和自己工号下的数据
			sqlstr = sqlstr +
				"  AND (USER_ID = ' ' OR USER_ID = @tps00p1.USER_ID) ";
		}
		sqlstr = sqlstr + "  ORDER BY  SEQ_NO ";
		Log::Trace("", __FUNCTION__, "===sqlstr =  [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tps00p1.USER_ID", tps00p1["USER_ID"].ToString());
		cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
		cmd_inq.Parameters.Set("tps00p1.PLAN_BACKLOG_CODE", tps00p1["PLAN_BACKLOG_CODE"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], 0, -1);
		
		/*设置系统返回参数*/
		CFormattable arguments[] = { bcls_ret->Tables[0].Rows.get_Count() };// 定义参数列表的数组
		CMessageFormat::Format(s.msg, _RES("GCRSS0000004")/*查询到[{0}]条记录。*/, arguments, 1);//格式化字符串
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)
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
