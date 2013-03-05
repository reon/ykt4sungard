/**
 * 
 */
package com.kingstargroup.ecard.portlet.advquery.action;

import java.util.Collections;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionServlet;
import org.apache.struts.action.DynaActionForm;

import com.kingstargroup.ecard.portlet.advquery.cewolf.CustomerDrawChart;
import com.kingstargroup.ecard.portlet.advquery.cewolf.DrawBarByTypeMap;
import com.kingstargroup.ecard.portlet.advquery.cewolf.DrawTimeSeries;
import com.kingstargroup.ecard.portlet.advquery.service.AdvqueryServiceUtil;
import com.kingstargroup.ecard.util.DateUtil;
import com.kingstargroup.ecard.util.EcardConstants;
import com.kingstargroup.ecard.util.ErrorInfo;
import com.kingstargroup.ecard.util.GetListResultStat;
import com.kingstargroup.ecard.util.GetProperty;
import com.kingstargroup.ecard.util.SortListByResult;
import com.liferay.util.servlet.SessionErrors;

/**
 * @author Administrator
 *
 */
public class QueryAllAccountChangeStatAction extends Action{

	/**
	 * ������������仯ͳ�ƣ���ֵ�����ѣ�Ѻ��
	 */
	public ActionForward execute(ActionMapping mapping, 
			ActionForm form, 
			HttpServletRequest request, 
			HttpServletResponse response) throws Exception {
		QueryAllAccountChangeStatActionExecution exec = new QueryAllAccountChangeStatActionExecution();
		return exec.execute(mapping,form,request,response);
	}
	private class QueryAllAccountChangeStatActionExecution {
		//ͳ�������Ĳ���
		private String statType ;
		//��ʼ����
		private String beginDate;
		//��������
		private String endDate;
		//���ز�ѯ���
		private List weekResult = null;
		//���ڷ�Χ
		private StringBuffer dateRange = new StringBuffer(200);
		//ѡ���ͼ����ʾ����
		private String showType;		
		private String picType = "";
		private String statTypeShow = "";
		//��λ
		private int dataUnit = 1;
		private int dataUnit2 = 1;
		private int dateUnit = 1;
		//������ʾ��Ϣ
		private String yLabel = "";
		private String yLabel2 = "";
		private String accType[] = {"���", "����", "Ѻ��"};
		private String accType2[] = {"Ѻ��(�Ŵ�)"};
		private String xData = "";
		private String dataType = "float";
		private String columnTitle[] = {"saving","consume","ensure"};
		private String columnTitle2[] = {"ensure"};
		
		// �Զ���ͼ����ʾҪ����Ĳ���
		// ��ʾ�Զ�����ɫ
		private boolean showCustColor = true;
		// ��ʾ���������ֵ���ת����
		private boolean showTextAxis = true;
		// ��ʾBarͼ���ϵ�����
		private boolean showLabel = false;
		// ��ʾ�Զ���ͼ��������ɫ
		private boolean showBackgroundColor = true;
		// ��ʾ������
		private boolean showOutLine = true;
		//��ʾ������
		private boolean showGrid = true;
		
		ActionServlet servlet = QueryAllAccountChangeStatAction.this.servlet;
		public ActionForward execute(ActionMapping mapping, ActionForm form,
				HttpServletRequest request, HttpServletResponse response)
			throws Exception{
			response.setCharacterEncoding("UTF-8");
			DynaActionForm allaccountform = (DynaActionForm) form;
			String querytype = allaccountform.getString("querytype");
			if(!"".equals(querytype)){
				allaccountform.set("querytype", "");
				return mapping.findForward("errortest");	
			}
			//ͳ�������Ĳ���
			statType = allaccountform.getString("statType");
			//��ʼ����
			beginDate = allaccountform.getString("allaccount_begindate");
			//��������
			endDate = allaccountform.getString("allaccount_enddate");
			//ѡ���ͼ����ʾ����
			showType = allaccountform.getString("showType");	
			if (("".equals(beginDate)) || ("".equals(endDate))) {
				SessionErrors.add(request, EcardConstants.ALLACCOUNT_ERROR_EXIST, ErrorInfo
						.getErrorTip(statType, beginDate, endDate));
				return mapping.findForward("errortest");
			}
			setInit(request);
			
			// �ж��Ƿ�������ͼ��ʾ
			String errtip = ErrorInfo.DateRangeJudeg(showType, statType,
					beginDate, endDate, request);
			if (!"".equals(errtip)) {
				SessionErrors.add(request, EcardConstants.ALLACCOUNT_ERROR_EXIST, errtip);
				return mapping.findForward("errortest");
			}
			
			List result = AdvqueryServiceUtil.getAllAccountChangeStat(beginDate,endDate,statType);
			if (result.isEmpty()) {
				SessionErrors.add(request, EcardConstants.ALLACCOUNT_ERROR_EXIST, ErrorInfo
						.getErrorTip(statType, beginDate, endDate));
				return mapping.findForward("errortest");
			}

			dateUnit = result.size()/10;
			setTypeReturn(request,result);
			
			//����Ҫ��ʾ��ͼ��������ͼ��
			showTypeResult();
			request.setAttribute("pictype",picType);
			request.setAttribute("ylabel",yLabel);
			request.setAttribute("ylabel2",yLabel2);
			request.setAttribute("stattype",statType);
			request.setAttribute("showtype",showType);
			request.setAttribute("rangedate",dateRange.toString());
			request.setAttribute("stattypeshow",statTypeShow);
			drawPic(request);
			return mapping.findForward("allaccount_queryresult");
		}
		
		/**
		 * ��ʼ��
		 * @param request
		 */
		private void setInit(HttpServletRequest request){
			if ("bydate".equals(statType)){
				beginDate = DateUtil.reFormatTime(beginDate);
				endDate = DateUtil.reFormatTime(endDate);	
				statTypeShow = GetProperty.getProperties("title.date",servlet.getServletContext());
				dateRange.append(GetProperty.getProperties("query.show.date",servlet.getServletContext()))
				.append(beginDate).append("-").append(endDate);
			}else if ("bymonth".equals(statType)){
				beginDate = DateUtil.getMonth(beginDate);
				endDate = DateUtil.getMonth(endDate);	
				statTypeShow = GetProperty.getProperties("title.month",servlet.getServletContext());
				dateRange.append(GetProperty.getProperties("query.show.date",servlet.getServletContext()))
				.append(beginDate).append("-").append(endDate);
			}else if ("byweek".equals(statType)){
				beginDate = DateUtil.reFormatTime(beginDate);
				endDate = DateUtil.reFormatTime(endDate);		
				statTypeShow = GetProperty.getProperties("title.week",servlet.getServletContext());
				dateRange.append(GetProperty.getProperties("query.show.week",servlet.getServletContext()))
				.append(beginDate).append("-").append(endDate);
			}
			
		}
		
		/**
		 * ����ѡ���ͳ�����ͻ���ͼ��
		 * @param request
		 * @param result
		 */
		private void setTypeReturn(HttpServletRequest request,List result){
			if ("bydate".equals(statType)){
				if ("bydate".equals(statType)){
					yLabel = new GetListResultStat().getYLabelMoney(result, "saving",dataType);
					dataUnit = new GetListResultStat().getUnitValue(result, "saving",dataType);
					yLabel2 = new GetListResultStat().getYLabelMoney(result, "ensure",dataType);
					dataUnit2 = new GetListResultStat().getUnitValue(result, "ensure",dataType);
					request.setAttribute("result", result);
					xData = "balance_date";
					if ("line".equals(showType)){
						request.setAttribute("allaccountstatChart",
						new DrawTimeSeries(result, columnTitle, xData,
								dataType, accType, dataUnit)
								.getTimeSeriesProducer());
						request.setAttribute("allaccountstatChart2",
								new DrawTimeSeries(result, columnTitle2, xData,
										dataType, accType2, dataUnit2)
										.getTimeSeriesProducer());

					}else{
						request.setAttribute("allaccountstatChart",
								new DrawBarByTypeMap(result,accType,xData,dataType,columnTitle,dataUnit)
								.getDataProducer());
					}
				}
			}else if ("bymonth".equals(statType)){
				request.setAttribute("result",result);
				yLabel = new GetListResultStat().getYLabelMoney(result,"saving",dataType);
				dataUnit = new GetListResultStat().getUnitValue(result,"saving",dataType);
				xData = "balance_date";
				request.setAttribute("allaccountstatChart" ,
						new DrawBarByTypeMap(result,accType,xData,dataType,columnTitle,dataUnit).getDataProducer());
				
				showTextAxis = ErrorInfo.showTextAxis(result);
			}else if ("byweek".equals(statType)){
				weekResult = new GetListResultStat().getListByWeek(result,beginDate,endDate);
				Collections.sort(weekResult,new SortListByResult("balance_date"));
				yLabel = new GetListResultStat().getYLabelMoney(weekResult,"saving",dataType);
				dataUnit = new GetListResultStat().getUnitValue(weekResult,"saving",dataType);
				xData = "yearweek";
				request.setAttribute("result",weekResult);
				request.setAttribute("allaccountstatChart" ,
						new DrawBarByTypeMap(weekResult,accType,xData,dataType,columnTitle,dataUnit).getDataProducer());
				
				showTextAxis = ErrorInfo.showTextAxis(weekResult);
			}
		}
		
		/**
		 * ����ѡ���ͼ����������ͼ������
		 *
		 */
		private void showTypeResult(){
			if ("line".equals(showType)) {
				if ("bydate".equals(statType)){
					picType = GetProperty.getProperties("pic.timeseries",servlet.getServletContext());
				}else{
					picType = GetProperty.getProperties("pic.line",servlet.getServletContext());
				}
				showOutLine = false;
			}else if ("bar".equals(showType)){
				picType = GetProperty.getProperties("pic.bar",servlet.getServletContext());
				showOutLine = true;
			}else if ("stack".equals(showType)){
				picType = GetProperty.getProperties("pic.stack",servlet.getServletContext());
				showOutLine = true;
			}
		}
		private void drawPic(HttpServletRequest request){
			if ("line".equals(showType)) {
				if ("bydate".equals(statType)){
					request.setAttribute("dataColor", new CustomerDrawChart(showBackgroundColor,
							showCustColor,dateUnit)
							.getTimeSeriesPostProcessor());
				}else{
					request.setAttribute("dataColor", new CustomerDrawChart(showCustColor,
							showTextAxis, showLabel, showBackgroundColor, showOutLine,
							showGrid).getChartPostProcessor());
				}
			}else if ("bar".equals(showType)) {
				request.setAttribute("dataColor", new CustomerDrawChart(showCustColor,
						showTextAxis, showLabel, showBackgroundColor, showOutLine,
						showGrid).getChartPostProcessor());
			}else if ("stack".equals(showType)) {
				request.setAttribute("dataColor", new CustomerDrawChart(showCustColor,
						showTextAxis, showLabel, showBackgroundColor, showOutLine,
						showGrid).getChartPostProcessor());
			}
		}
			
	}
}