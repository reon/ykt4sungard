/* --------------------------------------------
 * ��������: F849015.c
 * ��������: 2006-8-10
 * ��������: ����
 * �汾��Ϣ: 1.0.0.0
 * ������: ���ת��
 * --------------------------------------------
 * �޸�����:
 * �޸���Ա:
 * �޸�����:
 * �汾��Ϣ:
 * ��ע��Ϣ:
 * --------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpack.h"
#include "errdef.h"
#include "pubdef.h"
#include "pubdb.h"
#include "pubfunc.h"
#include "dbfunc.h"
#include "fdsqc.h"
#include "tcp.h"
#include "gwpack.h"
#include "account.h"
#include "dbfunc_foo.h"


static int process(InAcc *IA,T_t_tif_tradeserial *p)
{
	int ret = 0;

	//���ݿ��ź�Ǯ���ŵõ��������˺�(�跽)
	IA->iMainDevId=p->maindevice_id;			//����վ��ʶ
	IA->iDevId=p->device_id;					//�豸ID
	IA->iSerialNo=p->serial_no;					//��ˮ��
	IA->iTradeNo=p->serial_type;				//������
	strcpy(IA->sTxDate,p->operate_date);		//��������
	strcpy(IA->sTxTime,p->operate_time);		//����ʱ��
	strcpy(IA->sTxCollectDate,p->collect_date);	//�ɼ�����
	strcpy(IA->sTxCollectTime,p->collect_time);	//�ɼ�ʱ��
	strcpy(IA->sTxAccDate,p->enteract_date);	//��������
	strcpy(IA->sTxAccTime,p->enteract_time);	//����ʱ��
	strcpy(IA->sMdeOper,p->oper_code);		//¼�� ����Ա
	strcpy(IA->sChkOper,p->reserve_1);		//���� ����Ա

	IA->iTxCnt = p->trade_count;
	IA->iUseCardFlag=USE_CARD_TYPE_ONLINE;		//��������
	IA->dInCardBala=p->in_balance;					//�뿨ֵ
	IA->dOutCardBala=-1;					//����ֵ
	
	//�޸Ľ跽�ʹ����ʻ����ǻ�Ʒ�¼��
	ret=AccountProcess(IA);
	if(ret)
	{
		writelog(LOG_ERR,"AccountProcess ret[%d]",ret);
		return ret;
	}
	p->out_balance=IA->dOutCardBala;		//����ֵ
	return 0;
}
#ifdef __cplusplus
extern "C"
#endif
int F849015(TRUSERID *handle,int iRequest,ST_PACK *in_pack,int *pRetCode,char *szMsg)
{
	int ret=0;
	int i;
	int card_id = 0;
	char buf[101] = "";
	char sMsg[128] = "";
	char reqdata[1024] = "";
	char temp[1024] = "";
	char sysdate[9] = "";
	char systime[7] ="";
	T_t_tif_tradeserial tTradeSerial;
	T_t_aif_account tAccount,tShopAcc;
	T_t_pif_card tCard;
	T_t_cif_customer tCustomer;
	T_t_tradecode_shop tTradeShop;
	T_t_cif_shop tShop;
	gw_server_info_t svrinfo;
	char svr_ip[21 ] = "";
	int svr_port = 0;

	ST_CPACK aPack;
	ST_PACK* out_pack = &(aPack.pack);
	
	InAcc IA;
	double dUniNo;
	double fee,elec;

	memset(&tCard,0,sizeof tCard);
	memset(&tAccount,0,sizeof tAccount);
	memset(&tTradeSerial,0,sizeof tTradeSerial);
	memset(&IA,0,sizeof IA);
	memset(&tCustomer,0,sizeof tCustomer);
	memset(&tShop,0,sizeof tShop);
	memset(&tShopAcc,0,sizeof tShopAcc);

	CTcpSocket sock;
	KSG_GW_PACK_t * resp = NULL;

	// ������
	card_id = in_pack->lvol0;
	if(card_id < 1)
	{
		*pRetCode = E_INPUT_CARDNO_CANNOT_NULL;
		goto L_RETU;
	}

	if(amtcmp(in_pack->damt0,0) <= 0 || 
		amtcmp(in_pack->damt1,0) <= 0)
	{
		*pRetCode = E_INPUT_AMT;
		goto L_RETU;
	}

	if(strlen(in_pack->sname) < 1)
	{
		*pRetCode = E_INPUT_DORM_NO;
		goto L_RETU;
	}
	ret = DB_t_pif_card_read_by_card_id(card_id,&tCard);
	if(ret)
	{
		if(DB_NOTFOUND == ret)
			*pRetCode = E_DB_CARD_N;
		else
			*pRetCode = E_DB_CARD_R;
		goto L_RETU;
	}
	// ��鿨״̬
	if(strncmp(tCard.state_id,CARDSTAT_REG,4)!=0)
	{
		if('2'==tCard.state_id[CARDSTAT_TYPE_REG])
			ret= E_CARDNO_LOGOUT;
		else if('3'==tCard.state_id[CARDSTAT_TYPE_REG])
			ret = E_CARD_CHANGE;
		else if(tCard.state_id[CARDSTAT_TYPE_LOST]==STATE_TRUE)
			ret=E_CARDNO_LOST;
		else if(tCard.state_id[CARDSTAT_TYPE_FREEZE]==STATE_TRUE)
			ret=E_CARDNO_FREEZE;
		else if(tCard.state_id[CARDSTAT_TYPE_WFAIL]==STATE_TRUE)
			ret=E_CARDNO_WFAIL;
		*pRetCode = ret;
		goto L_RETU;
	}
	// ��ȡ�ʻ������Ϣ
	ret = DB_t_aif_account_read_by_card_id_and_purse_id(card_id,PURSE_NO_ONE,&tAccount);
	if(ret)
	{
		if(DB_NOTFOUND == ret)
			*pRetCode = E_DB_ACCOUNT_N;
		else
			*pRetCode = E_DB_ACCOUNT_R;
		goto L_RETU;
	}

	ret = DB_t_cif_customer_read_by_cut_id(tCard.cosumer_id,&tCustomer);
	if(ret)
	{
		if(DB_NOTFOUND == ret)
			*pRetCode = E_DB_CUSTOMER_N;
		else
			*pRetCode = E_DB_CUSTOMER_R;
		goto L_RETU;
	}

	ret = DB_t_tradecode_shop_read_by_trade_code(TXCODE_POWER_CONSUME,&tTradeShop);
	if(ret)
	{
		if(DB_NOTFOUND == ret)
			*pRetCode = E_DB_TRADECODE_SHOP_N;
		else
			*pRetCode = E_DB_TRADECODE_SHOP_R;
		goto L_RETU;
	}
	ret = DB_t_cif_shop_read_by_shop_id(tTradeShop.shop_id,&tShop);
	if(ret)
	{
		if(DB_NOTFOUND == ret)
			*pRetCode = E_DB_SHOP_N;
		else
			*pRetCode = E_DB_SHOP_R;
		goto L_RETU;
	}
	/*
	if('1' != tShop.is_leaf[0])
	{
		writelog(LOG_DEBUG,"��ذ��̻�[%d] ����Ҷ�ӽ��[%d]"
			,tTradeShop.shop_id,tShop.is_leaf[0]);
		*pRetCode = E_SHOP_ACTNO_NOT_EXIST;
		goto L_RETU;
	}
	*/
	ret = DB_t_aif_account_read_by_customer_id_and_act_type(
		tShop.cut_id,ACCTYPE_SHOPMAIN,&tShopAcc);
	if(ret)
	{
		if(DB_NOTFOUND == ret)
			*pRetCode = E_DB_ACCOUNT_N;
		else
			*pRetCode = E_DB_ACCOUNT_R;
		goto L_RETU;
	}
	
	ret = getNewUniqNo(KEYTYPE_TRADESERIAL,&dUniNo);
	if(ret)
	{
		*pRetCode = ret;
		goto L_RETU;
	}
	tTradeSerial.serial_no = D2I(dUniNo);
	tTradeSerial.serial_type = TXCODE_POWER_CONSUME;
	tTradeSerial.card_id = card_id;
	tTradeSerial.customer_id = tCard.cosumer_id;
	tTradeSerial.device_id = in_pack->lvol4;
	tTradeSerial.maindevice_id = in_pack->lvol3;
	tTradeSerial.in_balance = in_pack->damt1;
	tTradeSerial.trade_count = in_pack->lvol2 + 1;
	tTradeSerial.trade_fee = in_pack->damt0;
	tTradeSerial.serial_state = SERISTAT_DEBT;
	tTradeSerial.purse_id = PURSE_NO_ONE;
	des2src(tTradeSerial.oper_code,in_pack->scust_limit);
	
	getsysdate(sysdate);
	getsystime(systime);

	des2src(tTradeSerial.enteract_date,sysdate);
	des2src(tTradeSerial.enteract_time,systime);
	des2src(tTradeSerial.operate_date,sysdate);
	des2src(tTradeSerial.operate_time,systime);
	des2src(tTradeSerial.collect_date,sysdate);
	des2src(tTradeSerial.collect_time,systime);

	if(!(amtcmp(tAccount.cur_freebala,in_pack->damt0) > 0
		&& amtcmp(in_pack->damt1,in_pack->damt0)))
	{
		*pRetCode = E_BALANCE_SHORTAGE;
		goto L_RETU;
	}

	ResetNormalCPack(&aPack,0,1);
	SetCol(handle,0);
	SetCol(handle,F_LVOL0,F_DAMT0,F_DAMT1,F_VSMESS,0);
	
	ret = GetPowerServerInfo(svr_ip,&svr_port);
	if(ret)
	{
		*pRetCode = ret;
		goto L_RETU;
	}
	des2src(svrinfo.server_ip,svr_ip);
	svrinfo.server_port = svr_port;
	svrinfo.sock = &sock;

	AddXmlItemInt(temp,XML_KEY_SERIALNO,tTradeSerial.serial_no);
	AddXmlItemStr(temp,XML_KEY_NAME,tCustomer.cut_name);
	AddXmlItemDbl(temp,XML_KEY_AMT,in_pack->damt0);
	AddXmlItemStr(temp,XML_KEY_DATE,sysdate);
	AddXmlItemStr(temp,XML_KEY_TIME,systime);
	AddXmlItemStr(temp,XML_KEY_DORM_NO,in_pack->sname);
	AddXmlItemInt(temp,XML_KEY_CUSTID,tCard.cosumer_id);
	sprintf(reqdata,"<YKT_DATA>%s</YKT_DATA>",temp);
	*pRetCode  = 0;
	// ���û�е�������ģ��,����Ϊ����ֵ�����뿨ֵ
	tTradeSerial.out_balance = tTradeSerial.in_balance;
	ret = SendToServer(&svrinfo,TXCODE_POWER_CONSUME,reqdata);
	if(ret)
	{
		writelog(LOG_ERR,"���͵��ϵͳ����ʧ��");
		// �������ݲ��ɹ�., ֻ��¼��ˮ
		tTradeSerial.serial_state = SERISTAT_NODEBT;
		
	}
	else if((ret = RecvFromServer(&svrinfo,&resp)))
	{
		//��Ϊ���˳ɹ�,��Ҫ����
		writelog(LOG_ERR,"���ϵͳ��Ӧ��");
		tTradeSerial.serial_type = TXCODE_POWER_RUSH;
		tTradeSerial.serial_state = SERISTAT_NONEEDDEBT;
		ret = E_POWER_RECV_ERR;
	}
	else
	{
		if(resp->func_no != TXCODE_POWER_CONSUME || resp->retcode != 0)
		{
			writelog(LOG_ERR,"���ϵͳ���ش���[%d]",resp->retcode);
			// �������ʧ��
			ret = get_errcode_msg(resp->retcode,buf);
			if(!ret)
			{
				des2src(out_pack->vsmess,buf);
			}
			ret = E_POWER_RECV_ERR;
			tTradeSerial.serial_type = TXCODE_POWER_RUSH;
			tTradeSerial.serial_state = SERISTAT_NONEEDDEBT;
			
		}
		else
		{
			//������˳ɹ�
			des2src(reqdata,resp->data);
			if((ret = GetXmlValue(buf,sizeof buf,XML_KEY_AMT,reqdata)))
			{
				//��Ϊ���˳ɹ���Ҫ����
				writelog(LOG_ERR,"ת��ʧ��,��ط�����Ϣ����");
				tTradeSerial.serial_type = TXCODE_POWER_RUSH;
				tTradeSerial.serial_state = SERISTAT_NONEEDDEBT;
			}
			else if((ret = GetXmlValue(buf,sizeof buf,XML_KEY_ELEC,reqdata)))
			{
				//��Ϊ���˳ɹ�,��Ҫ����
				writelog(LOG_ERR,"ת��ʧ��,��ط�����Ϣ����");
				tTradeSerial.serial_type = TXCODE_POWER_RUSH;
				tTradeSerial.serial_state = SERISTAT_NONEEDDEBT;
			}
			else if((ret = GetXmlValue(buf,sizeof buf,XML_KEY_SERIALNO,reqdata)))
			{
				//��Ϊ���˳ɹ���Ҫ����
				writelog(LOG_ERR,"ת��ʧ��,��ط�����Ϣ����");
				tTradeSerial.serial_type = TXCODE_POWER_RUSH;
				tTradeSerial.serial_state = SERISTAT_NONEEDDEBT;
			}
			else
			{
				elec = atof(buf);
				fee = atof(buf);
				tTradeSerial.in_balance = in_pack->damt1;
				tTradeSerial.other_seri_no = D2I(atof(buf));
				IA.iCardNo = card_id;
				IA.dArrInAmt[0] = tTradeSerial.trade_fee;
				IA.dInCardBala = tTradeSerial.in_balance;
				IA.iFeeType = tCustomer.fee_type;
				des2src(IA.sArrInActno[0],tAccount.account_id); //�û��˺�
				des2src(IA.sArrInActno[1],tShopAcc.account_id); //�̻��˺�
				
				ret = process(&IA,&tTradeSerial);
				if(ret)
				{
					writelog(LOG_ERR,"�������ʧ��");
					// ���˲��ɹ�,��Ҫ����
					tTradeSerial.serial_type = TXCODE_POWER_RUSH;
					tTradeSerial.serial_state = SERISTAT_NONEEDDEBT;
				}
				else
				{
					out_pack->lvol0 = tTradeSerial.serial_no;
					out_pack->damt0 = tTradeSerial.out_balance;
					out_pack->damt1 = elec;
					// ������ϵͳ���صĵ���
					sprintf(tTradeSerial.reserve_1,"%.2lf",elec);
					sprintf(out_pack->vsmess,"��ˮ��:%d ����:%d ",IA.iSerialNo,IA.iCardNo);
					for(i=1;i<=IA.iOutTxTypeCnt;i++)
					{
						switch(IA.iArrOutTxType[i])
						{
							case TXTYPE_TOLL_DEPOSIT:
							case TXTYPE_TOLL_DEPOSIT_BILL:
							case TXTYPE_TOLL_DEPOSIT_FUNDBOOK:
							case TXTYPE_DEDUCT_DEPOSIT:
							case TXTYPE_RETURN_DEPOSIT:
								tTradeSerial.deposit_fee=IA.dArrOutAmt[i];
								break;
							case TXTYPE_PRE_TOLL_BOARD:
							case TXTYPE_PRE_TOLL_BOARD_BILL:
							case TXTYPE_PRE_TOLL_BOARD_FUNDBOOK:
							case TXTYPE_TOLL_BOARD:
							case TXTYPE_DEDUCT_BOARD:
							case TXTYPE_RETURN_BOARD:
							case TXTYPE_RETURN_BOARD_BILL:
							case TXTYPE_RETURN_BOARD_FUNDBOOK:
								tTradeSerial.boardfee=IA.dArrOutAmt[i];
								break;
							case TXTYPE_TOLL_CHARGE:
							case TXTYPE_TOLL_CHARGE_BILL:
							case TXTYPE_TOLL_CHARGE_FUNDBOOK:
								tTradeSerial.in_fee=IA.dArrOutAmt[i];
								break;
							case TXTYPE_TOLL_CARDCOST:
							case TXTYPE_TOLL_CARDCOST_BILL:
							case TXTYPE_TOLL_CARDCOST_FUNDBOOK:
								tTradeSerial.cost_fee=IA.dArrOutAmt[i];
								break;
							default:
								break;
						}
						if(amtcmp(IA.dArrOutAmt[i],0)!=0)
						{
							sprintf(sMsg,"%s:%.2lfԪ ",IA.sArrOutTxName[i],IA.dArrOutAmt[i]);
							strcat(out_pack->vsmess,sMsg);
						}
					}
					sprintf(sMsg,"����ǰ���:%.2lfԪ",tTradeSerial.out_balance);
					strcat(out_pack->vsmess,sMsg);
					writelog(LOG_DEBUG,"%s",out_pack->vsmess);
				}
			}
		}
	}
	if(ret)
		*pRetCode = ret;

	if(resp)
		free(resp);
	ret = DB_t_tif_tradeserial_add(&tTradeSerial);
	if(ret)
	{
		writelog(LOG_ERR,"���ת����ˮ��¼ʧ��serial_no[%d],serial_type[%d]",tTradeSerial.serial_no,tTradeSerial.serial_type);
		if(DB_REPEAT == ret)
			*pRetCode = E_DB_TRADESERIAL_E;
		else 
			*pRetCode = E_DB_TRADESERIAL_I;
		goto L_RETU;
	}
	if(*pRetCode != 0)
	{
		ret = db_commit();
		if(ret)
			*pRetCode = ret;
		goto L_RETU;
	}
	PutRow(handle,out_pack,pRetCode,szMsg);
	return 0;
L_RETU:
	return -1;
}
