/**
 * 
 */
package com.kingstargroup.ecard.portlet.distribution.action;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.PageContext;

import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;

import com.kingstargroup.ecard.exceptions.PortalException;
import com.kingstargroup.ecard.portlet.information.service.EcardInformationServiceUtil;
import com.kingstargroup.ecard.util.EcardConstants;
import com.liferay.portal.kernel.util.GetterUtil;
import com.liferay.portal.kernel.util.StringPool;
import com.liferay.portal.struts.ActionConstants;
import com.liferay.portal.struts.PortletAction;
import com.liferay.portal.util.WebKeys;
import com.liferay.util.servlet.SessionErrors;

/**
 * Copyright (C), 2000-2005, Kingstar Co., Ltd.
 * File name: AddDocumentAction.java0
 * Description: �����ĵ���Struts Actionʵ��
 * Modify History: 
 * ��������   ������   ����ʱ��     ��������
 * ===================================
 *  ����    Xiao Qi  2005-9-13  
 * @author Xiao Qi
 * @version 
 * @since 1.0
 */
public class AddServicePlaceAction extends PortletAction {
	/* (non-Javadoc)
	 * @see org.apache.struts.action.Action#execute(org.apache.struts.action.ActionMapping, org.apache.struts.action.ActionForm, javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)
	 */
	public ActionForward execute(
			ActionMapping mapping, ActionForm form, HttpServletRequest req,
			HttpServletResponse res)
		throws Exception {			
		try {
			String title = req.getParameter("title");
			String content = req.getParameter("content");
			String cmd = req.getParameter("cmd");		
			String creater = GetterUtil.get((String)req.getSession().getAttribute("j_screenname"), StringPool.BLANK);
//			String creater = GetterUtil.get((String)req.getSession().getAttribute("j_username"), StringPool.BLANK);
//			String creater = GetterUtil.get((String)req.getSession().getAttribute(WebKeys.USER_ID), StringPool.BLANK);
			if (isTokenValid(req)) {				
				EcardInformationServiceUtil.add(title, content, "", String.valueOf(EcardConstants.INFORMATION_DISTRIBUTION_TYPE), creater);
				resetToken(req);
			}
			//�����������Ϊ���沢������һ��������Ҫ�����Ӧ��Form����

			return mapping.findForward("portlet.serviceplace.view_admin");
		} catch (PortalException pe) {
			req.setAttribute(PageContext.EXCEPTION, pe);
			return mapping.findForward(ActionConstants.COMMON_ERROR);
		}
	}
}