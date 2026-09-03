/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		yanl
Version:    1.0
Date:		2018-08-02
Description:收池参数过滤_新增
**************************************************/

#include "stdafx.h"


// service入口
BM2F_ENTERACE(ps00p1_ins)
int f_ps00p1_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag = 0;
	int		newbk = 1;
	/* 业务变量 */
	CDecimal v_seq_no_max = 0;
	CDecimal v_cnt = 0;
	CString	 datetime = " ";			/* 取时间 */
	CString	 userid = " ";			/* 登陆用户 */

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
		tps00p1["TYPE_CODE"] = bcls_rec->Tables[0].Rows[0]["TYPE_CODE"];
		tps00p1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
		tps00p1["FILTER_ENAME"] = bcls_rec->Tables[0].Rows[0]["FILTER_ENAME"];
		tps00p1["FILTER_CNAME"] = bcls_rec->Tables[0].Rows[0]["FILTER_CNAME"];
		tps00p1["FILTER_STRING"] = bcls_rec->Tables[0].Rows[0]["FILTER_STRING"];
		tps00p1["FILTER_TEXT"] = bcls_rec->Tables[0].Rows[0]["FILTER_TEXT"];

		Log::Trace("", __FUNCTION__, "=== =  [{0}][{1}][{2}][{3}][{4}]"
			, tps00p1["FORM_NAME"].ToString(), tps00p1["PLAN_BACKLOG_CODE"].ToString(), tps00p1["USER_ID"].ToString(), tps00p1["TYPE_CODE"].ToString(), tps00p1["FILTER_ENAME"].ToString());
		Log::Trace("", __FUNCTION__, "===FILTER_STRING =  [{0}]", tps00p1["FILTER_STRING"].ToString());
		
		//获取当前画面，当前机组的最大顺序号
		sqlstr = "SELECT NVL(MAX(SEQ_NO),0)  FROM TPS00P1 "
			"  WHERE PLAN_BACKLOG_CODE = @tps00p1.PLAN_BACKLOG_CODE "
			"  AND FORM_NAME = @tps00p1.FORM_NAME  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
		cmd_inq.Parameters.Set("tps00p1.PLAN_BACKLOG_CODE", tps00p1["PLAN_BACKLOG_CODE"].ToString());
		v_seq_no_max = cmd_inq.ExecuteScalar();

		Log::Trace("", __FUNCTION__, "=== sqlstr =  [{0}]", sqlstr);
		Log::Trace("", __FUNCTION__, "=== v_seq_no_max =  [{0}]", v_seq_no_max);

		v_seq_no_max = v_seq_no_max + 1;
		if (tps00p1["FILTER_ENAME"].ToString().Trim() == "")
		{
			//过滤器编译后新增的过滤条件 
			tps00p1["FILTER_ENAME"] = v_seq_no_max.ToString();
		}
		else
		{
			//过滤代号检查
			sqlstr = "SELECT COUNT(PLAN_BACKLOG_CODE)  FROM TPS00P1 "
				"  WHERE PLAN_BACKLOG_CODE = @tps00p1.PLAN_BACKLOG_CODE "
				"  AND FORM_NAME = @tps00p1.FORM_NAME  "
				"  AND FILTER_ENAME = @tps00p1.FILTER_ENAME  "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tps00p1.FILTER_ENAME", tps00p1["FILTER_ENAME"].ToString());
			cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
			cmd_inq.Parameters.Set("tps00p1.PLAN_BACKLOG_CODE", tps00p1["PLAN_BACKLOG_CODE"].ToString());
			v_cnt = cmd_inq.ExecuteScalar(); 
			if (v_cnt > 0)
			{
				CFormattable arguments[] = { tps00p1["FILTER_ENAME"].ToString() }; 
				CMessageFormat::Format(s.msg, "当前画面，当前机组的过滤代号{0}]已存在，请修改过滤代号。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//过滤简称检查
			if (tps00p1["FILTER_CNAME"].ToString().Trim() != "")
			{
				sqlstr = "SELECT COUNT(PLAN_BACKLOG_CODE)  FROM TPS00P1 "
					"  WHERE PLAN_BACKLOG_CODE = @tps00p1.PLAN_BACKLOG_CODE "
					"  AND FORM_NAME = @tps00p1.FORM_NAME  "
					"  AND FILTER_CNAME = @tps00p1.FILTER_CNAME  "
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tps00p1.FILTER_CNAME", tps00p1["FILTER_CNAME"].ToString());
				cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
				cmd_inq.Parameters.Set("tps00p1.PLAN_BACKLOG_CODE", tps00p1["PLAN_BACKLOG_CODE"].ToString());
				v_cnt = cmd_inq.ExecuteScalar();
				if (v_cnt > 0)
				{
					CFormattable arguments[] = { tps00p1["FILTER_CNAME"].ToString() };
					CMessageFormat::Format(s.msg, "当前画面，当前机组的过滤简称{0}]已存在，请修改过滤代号。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}
		tps00p1["SEQ_NO"] = v_seq_no_max;
		tps00p1["REC_CREATOR"] = userid;
		tps00p1["REC_CREATE_TIME"] = datetime;
		tps00p1["REC_REVISOR"] = userid;
		tps00p1["REC_REVISE_TIME"] = datetime;
		tps00p1.TrimOrBlank();
		tps00p1.Insert();

				//设置返回列
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FILTER_ENAME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FILTER_STRING");
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[0]["FILTER_ENAME"] = tps00p1["FILTER_ENAME"];
		bcls_ret->Tables[0].Rows[0]["FILTER_STRING"] = tps00p1["FILTER_STRING"];
		Log::Trace("", __FUNCTION__, "===// FILTER_ENAME =  [{0}]", bcls_rec->Tables[0].Rows[0]["FILTER_ENAME"].ToString());

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
