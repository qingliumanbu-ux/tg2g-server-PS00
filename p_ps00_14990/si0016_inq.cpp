/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2012
Author:    yanlei
Version:   1.0
Date:      2012-05-23
Description: 工序、机组配置信息，查询
**************************************************/

/*<remark>=========================================================
/// <summary>
/// 工序、机组配置信息，查询
/// <para>获取产线类别。</para>
/// <para>查询当前产线类别的工序信息； </para>
/// <para>查询机组信息。(如果产线不空，获取已配置为当前产线及为配置的机组信息。) </para>
/// </summary>
/// <param name="tsi0001"> tsi0001 。</param>
/// <param name="tsi0015"> tsi0015 整表。</param>
/// <returns>无</returns>
===========================================================</remark>*/
#include "stdafx.h"

//程序用头文件



// service入口
BM2F_ENTERACE(si0016_inq)

int f_si0016_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	/* 程序内部变量 */
	int		doFlag =0;
	
	/* 业务变量 */ 
	CString sqlstr = "";
	CString	v_line_type =" ";/* 产线类型 */    

	/* 实体类定义 */
	CModel tsi0015("TSI0015");
	CModel tsi0001("TSI0001");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	try
	{ 
		//获取传入参数
		v_line_type = bcls_rec->Tables[0].Rows[0]["LINE_TYPE"];
		//获取工序信息-------------------------------------------------------------
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:		// Oracle 数据库			 
			default:
				sqlstr = "SELECT * "
						"   FROM TSI0001  "
						"  WHERE BACKLOG_TYPE = '10' ";
				if (v_line_type.Trim() != "")
				{
					sqlstr += "AND LINE_TYPE = @v_line_type ";
				}
					sqlstr +="  ORDER BY WHOLE_BACKLOG_CODE ASC";
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_line_type", v_line_type);
		cmd_inq.ExecuteReader();

		while(cmd_inq.Read())
		{
			cmd_inq.Fetch(tsi0001);
			tsi0001.TrimOrBlank();

			tsi0001.MergeTo(bcls_ret->Tables[0],false);
		}
		cmd_inq.Close();
		bcls_ret->Tables[0].set_TableName("TSI0001");

		//获取机组信息-------------------------------------------------------------
		bcls_ret->Tables.Add("TSI0015");
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:		// Oracle 数据库			 
			default:
				sqlstr = "SELECT * "
						"   FROM TSI0015  ";
				if (v_line_type.Trim() != "")
				{
					sqlstr += "WHERE (PS_BACKLOG_TYPE_CODE like @v_line_type||'%' OR PS_BACKLOG_TYPE_CODE = ' ') ";
				}
					sqlstr +=
						"  ORDER BY UNIT_CODE ASC ";
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_line_type", v_line_type);
		cmd_inq.ExecuteReader();

		while(cmd_inq.Read())
		{
			cmd_inq.Fetch(tsi0015);
			tsi0015.TrimOrBlank();

			tsi0015.MergeTo(bcls_ret->Tables[1],false);
		}
		cmd_inq.Close();
		
		_RES("EPESS0000001")/*查询成功。*/;
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}
