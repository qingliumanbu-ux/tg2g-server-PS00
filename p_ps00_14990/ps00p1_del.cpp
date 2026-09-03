/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		yanl
Version:    1.0
Date:		2018-08-02
Description:收池参数过滤_删除
**************************************************/

#include "stdafx.h"


// service入口
BM2F_ENTERACE(ps00p1_del)
int f_ps00p1_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag = 0;
	int		rowcnt = 0;
	/* 业务变量 */ 
	CDecimal v_cnt = 0;
	CString	 datetime = " ";			/* 取时间 */
	CString	 userid = " ";			/* 登陆用户 */
	CString  v_table = " ";
	CString	 v_remark = " ";

	/* 实体类定义 */
	CModel tps00p1("TPS00P1");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");	//取系统时间
		userid = s.userid;
		//-----------------------------------------------------------------------
		//获得输入参数
		tps00p1["USER_ID"] = s.userid;
		tps00p1["FORM_NAME"] = s.formname; 
		tps00p1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
		tps00p1["SEQ_NO"] = bcls_rec->Tables[0].Rows[0]["SEQ_NO"];

		Log::Trace("", __FUNCTION__, "=== =  [{0}][{1}][{2}][{3}] "
			, tps00p1["FORM_NAME"].ToString(), tps00p1["PLAN_BACKLOG_CODE"].ToString(), tps00p1["USER_ID"].ToString(), tps00p1["SEQ_NO"].ToString());
		rowcnt = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rowcnt; i++)
		{
			tps00p1["FORM_NAME"] = s.formname;
			tps00p1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[i]["PLAN_BACKLOG_CODE"];
			tps00p1["SEQ_NO"] = bcls_rec->Tables[0].Rows[i]["SEQ_NO"];

			Log::Trace("", __FUNCTION__, "=== =  [{0}][{1}][{2}][{3}] "
				, tps00p1["FORM_NAME"].ToString(), tps00p1["PLAN_BACKLOG_CODE"].ToString(), userid, tps00p1["SEQ_NO"].ToDecimal());

			if (tps00p1.Query("FORM_NAME,PLAN_BACKLOG_CODE,SEQ_NO") == false)
			{
				CFormattable arguments[] = { tps00p1["SEQ_NO"].ToDecimal() };
				CMessageFormat::Format(s.msg, "过滤（序号）{0}]未查询到数据。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tps00p1.Delete("FORM_NAME,PLAN_BACKLOG_CODE,SEQ_NO");

			//记录履历
			if (tps00p1["USER_ID"].ToString().Trim() == "")
			{
				v_remark = "删除公用过滤集，[";
			}
			else
			{
				v_remark = "删除定制过滤集，[";
			}
			v_remark = v_remark + tps00p1["SEQ_NO"].ToDecimal().ToString() + "],简称[" + tps00p1["FILTER_CNAME"].ToString() + "]过滤串[" + tps00p1["FILTER_STRING"].ToString() + "] ";
			v_table = tps00p1["FORM_NAME"].ToString().SubstringNE(2, 2);
			if (v_table == "CR"
				|| v_table == "HR"
				|| v_table == "HP")
			{
				sqlstr = "INSERT INTO TPS" + v_table + "99 ( "
					"		REC_CREATOR, REC_CREATE_TIME, PLAN_BACKLOG_CODE,PLAN_NO  "
					"		,FUNC_ID,PS_REMARK, SVC_NAME )"
					"	VALUES("
					"		@userid ,@datetime ,@tps00p1.PLAN_BACKLOG_CODE,@tps00p1.FORM_NAME "
					"		,'PS_DEL',@v_remark, 'ps00p1_del')"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("userid", userid);
				cmd_inq.Parameters.Set("datetime", datetime);
				cmd_inq.Parameters.Set("v_remark", v_remark);
				//cmd_inq.Parameters.Set("v_func_id", v_func_id);
				//cmd_inq.Parameters.Set("v_svc_name", v_svc_name);
				cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
				cmd_inq.Parameters.Set("tps00p1.PLAN_BACKLOG_CODE", tps00p1["PLAN_BACKLOG_CODE"].ToString());
				cmd_inq.ExecuteNonQuery();
			}
		}

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
