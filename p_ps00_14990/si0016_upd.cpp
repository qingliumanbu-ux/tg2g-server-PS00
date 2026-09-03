/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2012
Author:    yanlei
Version:   1.0
Date:      2012-05-23
Description: 工序、机组配置信息，配置设置（冷轧）
**************************************************/

/*<remark>=========================================================
/// <summary>
/// 工序、机组配置信息，配置设置（冷轧）
/// <para>获取工序，机组信息。</para>
/// <para>删除tsi0016当前工序的原对应机组； </para>
/// <para>在tsi0016中新增当前机组。 </para>
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
#include "stdafx.h"

//程序用头文件



// service入口
BM2F_ENTERACE(si0016_upd)

int f_si0016_upd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int       doFlag = 0;
	int		  RowNumer = 0;
	int		  RowCount = 0;

	/* 业务变量 */
	CString sqlstr = "";
	CString	datetime;				/* 取时间 */
	CString	userid = " ";			/* 登陆用户 */

	/* 实体类定义 */
	CModel tsi0001("TSI0001");
	CModel tsi0015("TSI0015");
	CDbCommand cmd_conn(conn);

	datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");	//取系统时间
	userid = s.userid;

	try
	{
		RowCount = bcls_rec->Tables[0].Rows.get_Count();

		////EDLog(1,1, "-- [%d]  ",RowCount);
		for (RowNumer = 0; RowNumer < RowCount; RowNumer++)
		{
			//获得输入参数
			tsi0001["WHOLE_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[RowNumer]["WHOLE_BACKLOG_CODE"];
			tsi0001["PS_BACKLOG_TYPE_CODE"] = bcls_rec->Tables[0].Rows[RowNumer]["PS_BACKLOG_TYPE_CODE"];
			tsi0001["PS_PLAN_SORT"] = bcls_rec->Tables[0].Rows[RowNumer]["PS_PLAN_SORT"];

			tsi0001.TrimOrBlank();
			tsi0001.Update("PS_BACKLOG_TYPE_CODE,PS_PLAN_SORT", "WHOLE_BACKLOG_CODE");

			//同时更新 TSI0015表
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库

			default: // 所有数据库适用，通用SQL语句
				sqlstr = "UPDATE  TSI0015 SET "
					"			PS_BACKLOG_TYPE_CODE = @ps_backlog_type_code, "
					"			PS_PLAN_SORT		 = @ps_plan_sort , "
					"			REC_REVISE_TIME = @datetime, "
					"			REC_REVISOR		= @userid  "
					"  WHERE  UNIT_CODE IN (SELECT DISTINCT UNIT_CODE  "
					"				FROM TSI0016 "
					"				WHERE WHOLE_BACKLOG_CODE = @whole_backlog_code) ";
				break;
			}
			cmd_conn.SetCommandText(sqlstr);
			cmd_conn.Parameters.Set("ps_backlog_type_code", tsi0001["PS_BACKLOG_TYPE_CODE"].ToString());
			cmd_conn.Parameters.Set("ps_plan_sort", tsi0001["PS_PLAN_SORT"].ToString());
			cmd_conn.Parameters.Set("whole_backlog_code", tsi0001["WHOLE_BACKLOG_CODE"].ToString());
			cmd_conn.Parameters.Set("datetime", datetime);
			cmd_conn.Parameters.Set("userid", userid);
			cmd_conn.ExecuteNonQuery();

			cmd_conn.Close();
		}

		//更新产出等待时间
		RowCount = 0;
		RowCount = bcls_rec->Tables[1].Rows.get_Count();
		for (RowNumer = 0; RowNumer < RowCount; RowNumer++)
		{
			//获得输入参数
			tsi0015["UNIT_CODE"] = bcls_rec->Tables[1].Rows[RowNumer]["UNIT_CODE"];
			tsi0015["STD_WAIT_TIME"] = bcls_rec->Tables[1].Rows[RowNumer]["STD_WAIT_TIME"];
			tsi0015["REC_REVISE_TIME"] = datetime;
			tsi0015["REC_REVISOR"] = s.userid;
			tsi0015.TrimOrBlank();
			tsi0015.Update("STD_WAIT_TIME,REC_REVISE_TIME,REC_REVISOR", "UNIT_CODE");
		}

		strcpy(s.msg, _RES("GCRSS0000002")/*处理成功。*/);

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

	return doFlag;

}
