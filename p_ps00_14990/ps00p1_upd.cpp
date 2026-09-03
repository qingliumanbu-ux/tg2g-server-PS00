/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		yanl
Version:    1.0
Date:		2018-08-02
Description:收池参数过滤_修改
**************************************************/

#include "stdafx.h"


// service入口
BM2F_ENTERACE(ps00p1_upd)
int f_ps00p1_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag = 0;
	int		rowcnt = 0;
	/* 业务变量 */
	CDecimal v_seq_no_max = 0;
	CDecimal v_cnt = 0;
	CString	 datetime = " ";			/* 取时间 */
	CString	 userid = " ";			/* 登陆用户 */
	CString	 v_update_type = " ";
	CString  v_table = " ";
	CString	 v_remark = " ";

	/* 实体类定义 */
	CModel tps00p1("TPS00P1");
	CModel tps00p1_pre("TPS00P1");

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
        //UPDATE_TYPE : 1 －filter_edite(只修改过滤串及本文)
        //              2 －filter_grid(修改 过滤代号、过滤简称、过滤工号)
		v_update_type = bcls_rec->Tables[0].Rows[0]["UPDATE_TYPE"];
		Log::Trace("", __FUNCTION__, "=== UPDATE_TYPE =  [{0}]", v_update_type);

		rowcnt = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rowcnt; i++)
		{
			tps00p1["FORM_NAME"] = s.formname;
			tps00p1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[i]["PLAN_BACKLOG_CODE"];
			tps00p1["SEQ_NO"] = bcls_rec->Tables[0].Rows[i]["SEQ_NO"];
			tps00p1["USER_ID"] = bcls_rec->Tables[0].Rows[i]["USER_ID"];
			tps00p1["FILTER_ENAME"] = bcls_rec->Tables[0].Rows[i]["FILTER_ENAME"];
			tps00p1["FILTER_CNAME"] = bcls_rec->Tables[0].Rows[i]["FILTER_CNAME"];
			tps00p1["FILTER_STRING"] = bcls_rec->Tables[0].Rows[i]["FILTER_STRING"];
			tps00p1["FILTER_TEXT"] = bcls_rec->Tables[0].Rows[i]["FILTER_TEXT"];
			tps00p1.TrimOrBlank();
			Log::Trace("", __FUNCTION__, "=== =  [{0}][{1}][{2}][{3}][{4}]"
				, tps00p1["FORM_NAME"].ToString(), tps00p1["PLAN_BACKLOG_CODE"].ToString(), tps00p1["USER_ID"].ToString(), tps00p1["TYPE_CODE"].ToString(), tps00p1["FILTER_ENAME"].ToString());
			Log::Trace("", __FUNCTION__, "===FILTER_STRING =  [{0}]", tps00p1["FILTER_STRING"].ToString());
			Log::Trace("", __FUNCTION__, "=== SEQ_NO =  [{0}]", tps00p1["SEQ_NO"].ToDecimal());

			if (v_update_type == "1")
			{
				//修改 过滤串及文本

				if (tps00p1["FILTER_ENAME"].ToString().Trim() =="")
				{
					sprintf(s.msg, "过滤代号为空，请联系系统维护人员。");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//获取原记录信息
				tps00p1_pre.Reset();
				tps00p1_pre["FORM_NAME"] = tps00p1["FORM_NAME"];
				tps00p1_pre["PLAN_BACKLOG_CODE"] = tps00p1["PLAN_BACKLOG_CODE"];
				tps00p1_pre["FILTER_ENAME"] = tps00p1["FILTER_ENAME"];
				tps00p1_pre.Query("FORM_NAME,PLAN_BACKLOG_CODE,FILTER_ENAME");

				if (tps00p1["FILTER_STRING"].ToString().Trim() == "")
				{
					sprintf(s.msg, "过滤条件串为空，不能修改。");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (tps00p1["FILTER_TEXT"].ToString().Trim() == "")
				{
					sprintf(s.msg, "过滤显示文本，不能修改。");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//修改数据
				tps00p1["REC_REVISOR"] = userid;
				tps00p1["REC_REVISE_TIME"] = datetime;
				tps00p1.Update("FILTER_STRING,FILTER_TEXT,REC_REVISOR,REC_REVISE_TIME", "FORM_NAME,PLAN_BACKLOG_CODE,SEQ_NO");

				// 修改履历
				v_remark = tps00p1_pre["SEQ_NO"].ToDecimal().ToString() + ",修改过滤串，原数据：" + tps00p1_pre["FILTER_STRING"].ToString();
			}
			else if (v_update_type == "2")
			{
				//修改 过滤代号、过滤简称、过滤工号

				if (tps00p1["SEQ_NO"].ToDecimal() < 0)
				{
					sprintf(s.msg, "顺序号为空，请联系系统维护人员。");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//获取原记录信息
				tps00p1_pre.Reset();
				tps00p1_pre["FORM_NAME"] = tps00p1["FORM_NAME"];
				tps00p1_pre["PLAN_BACKLOG_CODE"] = tps00p1["PLAN_BACKLOG_CODE"];
				tps00p1_pre["SEQ_NO"] = tps00p1["SEQ_NO"];
				tps00p1_pre.Query("FORM_NAME,PLAN_BACKLOG_CODE,SEQ_NO");

				if (tps00p1["FILTER_ENAME"].ToString().Trim() == "")
				{
					sprintf(s.msg, "过滤代号为空，不能修改。");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//过滤代号检查
				sqlstr = "SELECT COUNT(PLAN_BACKLOG_CODE)  FROM TPS00P1 "
					"  WHERE PLAN_BACKLOG_CODE = @tps00p1.PLAN_BACKLOG_CODE "
					"  AND FORM_NAME = @tps00p1.FORM_NAME  "
					"  AND FILTER_ENAME = @tps00p1.FILTER_ENAME  "
					"  AND SEQ_NO <> @tps00p1.SEQ_NO  "
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tps00p1.FILTER_ENAME", tps00p1["FILTER_ENAME"].ToString());
				cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
				cmd_inq.Parameters.Set("tps00p1.SEQ_NO", tps00p1["SEQ_NO"].ToDecimal());
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
						"  AND SEQ_NO <> @tps00p1.SEQ_NO  "
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tps00p1.FILTER_CNAME", tps00p1["FILTER_CNAME"].ToString());
					cmd_inq.Parameters.Set("tps00p1.FORM_NAME", tps00p1["FORM_NAME"].ToString());
					cmd_inq.Parameters.Set("tps00p1.SEQ_NO", tps00p1["SEQ_NO"].ToDecimal());
					cmd_inq.Parameters.Set("tps00p1.PLAN_BACKLOG_CODE", tps00p1["PLAN_BACKLOG_CODE"].ToString());
					v_cnt = cmd_inq.ExecuteScalar();
					if (v_cnt > 0)
					{
						CFormattable arguments[] = { tps00p1["FILTER_CNAME"].ToString() };
						CMessageFormat::Format(s.msg, "当前画面，当前机组的过滤简称{0}]已存在，请修改过滤代号。", arguments, 1);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}

				//修改数据
				tps00p1["REC_REVISOR"] = userid;
				tps00p1["REC_REVISE_TIME"] = datetime;
				tps00p1.Update("FILTER_ENAME,FILTER_CNAME,USER_ID,REC_REVISOR,REC_REVISE_TIME", "FORM_NAME,PLAN_BACKLOG_CODE,SEQ_NO");

				// 修改履历
				v_remark = tps00p1_pre["SEQ_NO"].ToDecimal().ToString() + " ";
				if (tps00p1["USER_ID"].ToString() != tps00p1_pre["USER_ID"].ToString())
				{
					v_remark = ",修改【工号】，原数据：" + tps00p1_pre["USER_ID"].ToString() + "新数据：" + tps00p1["USER_ID"].ToString();
				}
				if (tps00p1["FILTER_ENAME"].ToString() != tps00p1_pre["FILTER_ENAME"].ToString())
				{
					v_remark = ",修改【过滤代号】，原数据：" + tps00p1_pre["FILTER_ENAME"].ToString() + "新数据：" + tps00p1["FILTER_ENAME"].ToString();
				}
				if (tps00p1["FILTER_CNAME"].ToString() != tps00p1_pre["FILTER_CNAME"].ToString())
				{
					v_remark = ",修改【过滤简称】，原数据：" + tps00p1_pre["FILTER_CNAME"].ToString() + "新数据：" + tps00p1["FILTER_CNAME"].ToString();
				}
			}
			else
			{
				CFormattable arguments[] = { v_update_type };
				CMessageFormat::Format(s.msg, "更新类型{0}]有误，请联系系统维护人员。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}



			//记录履历
			//CString v_func_id = "PS_UPD";
			//CString v_svc_name = "ps00p1_upd";
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
					"		,'PS_UPD',@v_remark, 'ps00p1_upd')"
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
