/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2012
Author:    yanlei
Version:   1.0
Date:      2012-05-23
Description: 工序、机组配置信息，查询
**************************************************/

/*<remark>=========================================================
/// <summary>
/// 工序、机组配置信息，关联机组
/// <para>获取工序，机组信息。</para>
/// <para>删除tsi0016当前工序的原对应机组； </para>
/// <para>在tsi0016中新增当前机组。 </para>
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
#include "stdafx.h"

//程序用头文件


// service入口
BM2F_ENTERACE(si0016_f5)

int f_si0016_f5(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag =0;
	int		RowNumer = 0;
	int		RowCount = 0;
	
	/* 业务变量 */ 
	CString sqlstr = "";
	CString	datetime ;				/* 取时间 */
	CString	userid =" ";			/* 登陆用户 */ 

	/* 实体类定义 */
	CModel tsi0016("TSI0016");

	datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");	//取系统时间
	userid = s.userid;

	try
	{ 
		tsi0016["WHOLE_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["WHOLE_BACKLOG_CODE"];
		tsi0016["WHOLE_BACKLOG_NAME"] = bcls_rec->Tables[0].Rows[0]["WHOLE_BACKLOG_NAME"];

		tsi0016.Delete("WHOLE_BACKLOG_CODE");

		tsi0016["REC_CREATE_TIME"] = datetime;
		tsi0016["REC_CREATOR"] = s.userid;

		RowCount = bcls_rec->Tables[1].Rows.get_Count();
		for (RowNumer = 1; RowNumer <= RowCount ;RowNumer++ ) 
		{	
			//获得输入参数
			tsi0016["UNIT_CODE"]  = bcls_rec->Tables[1].Rows[RowNumer-1]["UNIT_CODE"];

			tsi0016.TrimOrBlank();
			tsi0016.Insert();
		}

		strcpy(s.msg,_RES("GCRSS0000002")/*处理成功。*/);
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
